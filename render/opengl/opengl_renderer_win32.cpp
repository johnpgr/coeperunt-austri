#include "opengl_renderer.h"
#include "../../platform/win32.h"

// ============================================================================
// OpenGL X-Macro Function Pointer Instantiation
// ============================================================================
#define X(name, ret, params) PFN_##name name = nullptr;
OPENGL_PROC_LIST
#undef X

// ============================================================================
// Modern WGL Extensions Function Pointer Instantiation
// ============================================================================
typedef BOOL (WINAPI *PFN_wglChoosePixelFormatARB)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC (WINAPI *PFN_wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL (WINAPI *PFN_wglSwapIntervalEXT)(int interval);

static PFN_wglChoosePixelFormatARB wglChoosePixelFormatARB = nullptr;
static PFN_wglCreateContextAttribsARB wglCreateContextAttribsARB = nullptr;
static PFN_wglSwapIntervalEXT wglSwapIntervalEXT = nullptr;

// ============================================================================
// GDI, User32 and standard WGL Procedure Pointers
// ============================================================================
typedef HGLRC (__stdcall *PFN_wglCreateContext)(HDC);
typedef BOOL (__stdcall *PFN_wglDeleteContext)(HGLRC);
typedef BOOL (__stdcall *PFN_wglMakeCurrent)(HDC, HGLRC);
typedef PROC (__stdcall *PFN_wglGetProcAddress)(LPCSTR);

static PFN_wglCreateContext wglCreateContext = nullptr;
static PFN_wglDeleteContext wglDeleteContext = nullptr;
static PFN_wglMakeCurrent wglMakeCurrent = nullptr;
static PFN_wglGetProcAddress wglGetProcAddress_ptr = nullptr;

typedef HDC (__stdcall *PFN_GetDC)(HWND);
typedef int (__stdcall *PFN_ReleaseDC)(HWND, HDC);
typedef int (__stdcall *PFN_ChoosePixelFormat)(HDC, const PIXELFORMATDESCRIPTOR*);
typedef BOOL (__stdcall *PFN_SetPixelFormat)(HDC, int, const PIXELFORMATDESCRIPTOR*);
typedef BOOL (__stdcall *PFN_SwapBuffers)(HDC);

static PFN_GetDC GetDC_ptr = nullptr;
static PFN_ReleaseDC ReleaseDC_ptr = nullptr;
static PFN_ChoosePixelFormat ChoosePixelFormat_ptr = nullptr;
static PFN_SetPixelFormat SetPixelFormat_ptr = nullptr;
static PFN_SwapBuffers SwapBuffers_ptr = nullptr;

// ============================================================================
// Dynamic OpenGL & GDI DLL Resolvers
// ============================================================================
static HMODULE g_opengl_lib = nullptr;

static void opengl_load_symbols() noexcept {
    if (g_opengl_lib != nullptr) return; // Already loaded

    g_opengl_lib = LoadLibraryA("opengl32.dll");
    if (!g_opengl_lib) {
        core::printf("[Error] Failed to load opengl32.dll\n");
        return;
    }

#ifdef PLATFORM_LOAD
#undef PLATFORM_LOAD
#endif

#define PLATFORM_LOAD(dll, name) \
    name = (PFN_##name)GetProcAddress(dll, #name)

#define PLATFORM_LOAD_PTR(dll, name, ptr) \
    ptr = (PFN_##name)GetProcAddress(dll, #name)

    PLATFORM_LOAD(g_opengl_lib, wglCreateContext);
    PLATFORM_LOAD(g_opengl_lib, wglDeleteContext);
    PLATFORM_LOAD(g_opengl_lib, wglMakeCurrent);
    PLATFORM_LOAD_PTR(g_opengl_lib, wglGetProcAddress, wglGetProcAddress_ptr);

    HMODULE user32_lib = LoadLibraryA("user32.dll");
    HMODULE gdi32_lib = LoadLibraryA("gdi32.dll");

    PLATFORM_LOAD_PTR(user32_lib, GetDC, GetDC_ptr);
    PLATFORM_LOAD_PTR(user32_lib, ReleaseDC, ReleaseDC_ptr);
    
    PLATFORM_LOAD_PTR(gdi32_lib, ChoosePixelFormat, ChoosePixelFormat_ptr);
    PLATFORM_LOAD_PTR(gdi32_lib, SetPixelFormat, SetPixelFormat_ptr);
    PLATFORM_LOAD_PTR(gdi32_lib, SwapBuffers, SwapBuffers_ptr);

#undef PLATFORM_LOAD
#undef PLATFORM_LOAD_PTR
}

static void opengl_load_procedures() noexcept {
    if (glGenBuffers != nullptr) return; // Already loaded

    // Load standard and modern GL procedures
    #define X(name, ret, params) \
        if (wglGetProcAddress_ptr) { \
            name = (PFN_##name)wglGetProcAddress_ptr(#name); \
        } \
        if (name == nullptr) { \
            name = (PFN_##name)GetProcAddress(g_opengl_lib, #name); \
        }
    OPENGL_PROC_LIST
    #undef X
}

// ============================================================================
// Pixel Format & Context Constants
// ============================================================================
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_TYPE_RGBA_ARB                         0x202B
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2018
#define WGL_STENCIL_BITS_ARB                      0x2023

// ============================================================================
// Global Instance Quad Buffer
// ============================================================================
#define MAX_QUADS 10000
static RenderCommandQuad g_quad_instances[MAX_QUADS];

static GLuint compile_shader(GLenum type, const char* version_header, const char* source) noexcept {
    GLuint shader = glCreateShader(type);
    const char* sources[2] = { version_header, source };
    glShaderSource(shader, 2, sources, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, nullptr, buffer);
        core::printf("[Error] OpenGL shader compilation error: %s\n", buffer);
        return 0;
    }
    return shader;
}

static GLuint link_program(GLuint vs, GLuint fs) noexcept {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetProgramInfoLog(program, 512, nullptr, buffer);
        core::printf("[Error] OpenGL program linking error: %s\n", buffer);
        return 0;
    }
    return program;
}

// ============================================================================
// OpenGL Win32 Interface Implementations
// ============================================================================
static GraphicsContext g_context = {};

EXTERN_C GraphicsContext* __opengl_init_graphics(PlatformWindow* window) noexcept {
    if (!window) return nullptr;

    core::printf("[System] OpenGL Win32 renderer init start.\n");

    // Load libraries
    opengl_load_symbols();

    HWND hwnd = *(HWND*)window;
    i32 width = *(i32*)((char*)window + sizeof(HWND));
    i32 height = *(i32*)((char*)window + sizeof(HWND) + sizeof(i32));

    // 1. Bootstrapping Window and Context to load modern WGL extensions
    WNDCLASSEXA dummy_wc = {};
    dummy_wc.cbSize = sizeof(dummy_wc);
    dummy_wc.style = CS_HREDRAW | CS_VREDRAW;
    dummy_wc.lpfnWndProc = win32::DefWindowProcA;
    dummy_wc.hInstance = GetModuleHandleA(nullptr);
    dummy_wc.lpszClassName = "GLDummyWindowClass";
    win32::RegisterClassExA(&dummy_wc);

    HWND dummy_hwnd = win32::CreateWindowExA(
        0, dummy_wc.lpszClassName, "Dummy",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, dummy_wc.hInstance, nullptr
    );

    HDC dummy_dc = GetDC_ptr(dummy_hwnd);

    PIXELFORMATDESCRIPTOR dummy_pfd = {};
    dummy_pfd.nSize = sizeof(dummy_pfd);
    dummy_pfd.nVersion = 1;
    dummy_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    dummy_pfd.iPixelType = PFD_TYPE_RGBA;
    dummy_pfd.cColorBits = 32;
    dummy_pfd.cDepthBits = 24;
    dummy_pfd.cStencilBits = 8;
    dummy_pfd.iLayerType = PFD_MAIN_PLANE;

    int dummy_pf = ChoosePixelFormat_ptr(dummy_dc, &dummy_pfd);
    SetPixelFormat_ptr(dummy_dc, dummy_pf, &dummy_pfd);

    HGLRC dummy_rc = wglCreateContext(dummy_dc);
    wglMakeCurrent(dummy_dc, dummy_rc);

    // Resolve extension pointers
    wglChoosePixelFormatARB = (PFN_wglChoosePixelFormatARB)wglGetProcAddress_ptr("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFN_wglCreateContextAttribsARB)wglGetProcAddress_ptr("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFN_wglSwapIntervalEXT)wglGetProcAddress_ptr("wglSwapIntervalEXT");

    // Release bootstrapping window
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(dummy_rc);
    ReleaseDC_ptr(dummy_hwnd, dummy_dc);
    win32::DestroyWindow(dummy_hwnd);

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        core::printf("[Error] Failed to load modern WGL extensions.\n");
        return nullptr;
    }

    // 2. Setup OpenGL Context on Real Window
    HDC real_dc = GetDC_ptr(hwnd);

    int pixel_format = 0;
    UINT num_formats = 0;
    int pf_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };

    wglChoosePixelFormatARB(real_dc, pf_attribs, nullptr, 1, &pixel_format, &num_formats);

    PIXELFORMATDESCRIPTOR real_pfd = {};
    real_pfd.nSize = sizeof(real_pfd);
    real_pfd.nVersion = 1;
    real_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    real_pfd.iPixelType = PFD_TYPE_RGBA;
    real_pfd.cColorBits = 32;
    real_pfd.cDepthBits = 24;
    real_pfd.cStencilBits = 8;
    real_pfd.iLayerType = PFD_MAIN_PLANE;

    SetPixelFormat_ptr(real_dc, pixel_format, &real_pfd);

    struct GLVersion {
        int major;
        int minor;
    };
    GLVersion fallback_versions[] = {
        {4, 6},
        {4, 3},
        {4, 1},
        {3, 3}
    };

    HGLRC real_rc = nullptr;
    int major_version = 0;
    int minor_version = 0;

    for (int i = 0; i < 4; ++i) {
        int context_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, fallback_versions[i].major,
            WGL_CONTEXT_MINOR_VERSION_ARB, fallback_versions[i].minor,
            WGL_CONTEXT_FLAGS_ARB, 0,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        real_rc = wglCreateContextAttribsARB(real_dc, nullptr, context_attribs);
        if (real_rc) {
            major_version = fallback_versions[i].major;
            minor_version = fallback_versions[i].minor;
            break;
        }
    }

    if (!real_rc) {
        core::printf("[Error] Failed to create OpenGL Context (tried 4.6 down to 3.3).\n");
        ReleaseDC_ptr(hwnd, real_dc);
        return nullptr;
    }

    wglMakeCurrent(real_dc, real_rc);

    // Load modern and standard OpenGL procedures now that a modern context is active
    opengl_load_procedures();

    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(1); // Enable VSync
    }

    core::printf("[System] OpenGL %d.%d Core Profile Context initialized.\n", major_version, minor_version);

    // 3. Read shader files from disk dynamically using custom Path and MemoryArena
    MemoryArena temp_arena = arena_alloc(Megabytes(1)); // 1MB temp arena for shaders
    if (!temp_arena.base) {
        core::printf("[Error] Failed to allocate temporary arena for shaders!\n");
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(real_rc);
        ReleaseDC_ptr(hwnd, real_dc);
        return nullptr;
    }

    Path vs_path = path_parse(&temp_arena, str8_lit("assets/shaders/quads.vert"));
    Path fs_path = path_parse(&temp_arena, str8_lit("assets/shaders/quads.frag"));

    String8 vs_content = api.fs.read_entire_file(&temp_arena, vs_path);
    String8 fs_content = api.fs.read_entire_file(&temp_arena, fs_path);

    if (vs_content.size == 0 || fs_content.size == 0) {
        core::printf("[Error] Failed to load OpenGL shader files from disk!\n");
        arena_release(&temp_arena);
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(real_rc);
        ReleaseDC_ptr(hwnd, real_dc);
        return nullptr;
    }

    // Build the GLSL version header dynamically based on the initialized context version
    char version_header[64];
    core::sprintf(version_header, "#version %d%d0 core\n", major_version, minor_version);

    // Compile and link shaders
    GLuint vs = compile_shader(GL_VERTEX_SHADER, version_header, (const char*)vs_content.str);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, version_header, (const char*)fs_content.str);
    GLuint program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Free the temporary arena and raw shader memory
    arena_release(&temp_arena);

    if (!program) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(real_rc);
        ReleaseDC_ptr(hwnd, real_dc);
        return nullptr;
    }

    // Setup buffers (VAO + VBO)
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * sizeof(RenderCommandQuad), nullptr, GL_DYNAMIC_DRAW);

    // Map attributes matching RenderCommandQuad:
    // Location 0: pos (float2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)0);
    glVertexAttribDivisor(0, 1);

    // Location 1: scale (float2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(1, 1);

    // Location 2: origin (float2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)(4 * sizeof(float)));
    glVertexAttribDivisor(2, 1);

    // Location 3: uv_pos (float2)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    // Location 4: uv_size (float2)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)(8 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    // Location 5: rotation (float)
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)(10 * sizeof(float)));
    glVertexAttribDivisor(5, 1);

    // Location 6: color (float4)
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(RenderCommandQuad), (void*)(11 * sizeof(float)));
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);

    // Create main texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    g_context.program = program;
    g_context.vao = vao;
    g_context.vbo = vbo;
    g_context.texture_id = texture_id;
    g_context.cached_width = width;
    g_context.cached_height = height;
    g_context.hwnd = hwnd;
    g_context.hdc = real_dc;
    g_context.hglrc = real_rc;

    return &g_context;
}

EXTERN_C b8 __opengl_upload_texture(GraphicsContext* context, const u8* pixels, i32 width, i32 height) noexcept {
    if (!context || !pixels) return false;

    glBindTexture(GL_TEXTURE_2D, context->texture_id);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        width, height, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, pixels
    );
    return true;
}

EXTERN_C void __opengl_submit_frame(PlatformWindow* window, GraphicsContext* context, RenderCommandQueue queue) noexcept {
    if (!context) return;

    // Retrieve cached window dimensions
    i32 width = *(i32*)((char*)window + sizeof(HWND));
    i32 height = *(i32*)((char*)window + sizeof(HWND) + sizeof(i32));
    context->cached_width = width;
    context->cached_height = height;

    // 1. Process batch render queue commands
    usize offset = 0;
    i32 quad_count = 0;

    while (offset < queue.size) {
        RenderCommandHeader* header = (RenderCommandHeader*)(queue.buffer + offset);
        
        switch (header->kind) {
            case RENDER_COMMAND_CLEAR: {
                RenderCommandClear* clear_cmd = (RenderCommandClear*)(queue.buffer + offset + sizeof(RenderCommandHeader));
                glClearColor(clear_cmd->r, clear_cmd->g, clear_cmd->b, clear_cmd->a);
                glClear(GL_COLOR_BUFFER_BIT);
            } break;
            
            case RENDER_COMMAND_QUAD: {
                RenderCommandQuad* quad_cmd = (RenderCommandQuad*)(queue.buffer + offset + sizeof(RenderCommandHeader));
                if (quad_count < MAX_QUADS) {
                    g_quad_instances[quad_count++] = *quad_cmd;
                }
            } break;
        }
        
        offset += header->size;
    }

    // 2. Draw batched instanced quads
    if (quad_count > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, context->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, quad_count * sizeof(RenderCommandQuad), g_quad_instances);

        glUseProgram(context->program);
        glBindVertexArray(context->vao);

        glActiveTexture(0x84C0); // GL_TEXTURE0
        glBindTexture(GL_TEXTURE_2D, context->texture_id);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, width, height);

        // Flipped Y orthographic matrix so Y points downwards
        f32 projection[16] = {0};
        projection[0] = 2.0f / (f32)width;
        projection[5] = -2.0f / (f32)height;
        projection[10] = 1.0f;
        projection[12] = -1.0f;
        projection[13] = 1.0f;
        projection[15] = 1.0f;

        GLint proj_loc = glGetUniformLocation(context->program, "projection");
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, projection);

        // Draw quad instances (6 vertices per instance)
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, quad_count);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    // 3. Swap back buffers
    SwapBuffers_ptr((HDC)context->hdc);
}

EXTERN_C void __opengl_destroy_graphics(GraphicsContext* context) noexcept {
    if (context) {
        if (context->vao) {
            glDeleteVertexArrays(1, &context->vao);
            context->vao = 0;
        }
        if (context->vbo) {
            glDeleteBuffers(1, &context->vbo);
            context->vbo = 0;
        }
        if (context->texture_id) {
            glDeleteTextures(1, &context->texture_id);
            context->texture_id = 0;
        }
        if (context->program) {
            glDeleteProgram(context->program);
            context->program = 0;
        }
        if (context->hglrc) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(context->hglrc);
            context->hglrc = nullptr;
        }
        if (context->hdc) {
            ReleaseDC_ptr((HWND)context->hwnd, (HDC)context->hdc);
            context->hdc = nullptr;
        }
    }
}
