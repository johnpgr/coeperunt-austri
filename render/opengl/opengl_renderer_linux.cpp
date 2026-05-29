#include "opengl_renderer.h"
#include "glx/glx_linux.h"

// ============================================================================
// OpenGL X-Macro Function Pointer Instantiation
// ============================================================================
#define X(name, ret, params) PFN_##name name = nullptr;
OPENGL_PROC_LIST
#undef X

static void opengl_load_symbols() noexcept {
    if (glGenBuffers != nullptr) return; // Already loaded

    if (!glx_load_symbols()) {
        core::printf("[Error] GLX symbols not loaded.\n");
        return;
    }

    #define X(name, ret, params) \
        if (glXGetProcAddress_ptr) { \
            name = (PFN_##name)glXGetProcAddress_ptr(#name); \
        } \
        if (name == nullptr) { \
            name = (PFN_##name)glx_dlsym(#name); \
        }
    OPENGL_PROC_LIST
    #undef X
}

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
// OpenGL Linux GLX Interface Implementations
// ============================================================================
static GraphicsContext g_context = {};

EXTERN_C GraphicsContext* __opengl_init_graphics(PlatformWindow* window) noexcept {
    if (!window) return nullptr;

    core::printf("[System] OpenGL Linux renderer init start.\n");

    // Load dynamic libraries
    opengl_load_symbols();
    if (!glXChooseVisual || !glXCreateContext || !glXMakeCurrent || !glXSwapBuffers || !glXDestroyContext) {
        core::printf("[Error] GLX symbols not loaded.\n");
        return nullptr;
    }

    Display* display = *(Display**)window;
    Window win = *(Window*)((char*)window + sizeof(Display*));
    i32 width = *(i32*)((char*)window + sizeof(Display*) + sizeof(Window) + sizeof(Atom));
    i32 height = *(i32*)((char*)window + sizeof(Display*) + sizeof(Window) + sizeof(Atom) + sizeof(i32));

    // Select visual configuration
    int visual_attribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo* vi = glXChooseVisual(display, DefaultScreen(display), visual_attribs);
    if (!vi) {
        core::printf("[Error] glXChooseVisual failed.\n");
        return nullptr;
    }

    GLXContext glx_context = glXCreateContext(display, vi, nullptr, GL_TRUE);
    if (!glx_context) {
        core::printf("[Error] glXCreateContext failed.\n");
        return nullptr;
    }

    glXMakeCurrent(display, win, glx_context);

    core::printf("[System] GLX OpenGL context initialized.\n");

    // Query OpenGL version details
    GLint major_version = 0;
    GLint minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);

    if (major_version < 3) {
        major_version = 3;
        minor_version = 3;
    }

    core::printf("[System] OpenGL %d.%d Context initialized.\n", major_version, minor_version);

    // Read shader files dynamically using custom Path and MemoryArena
    MemoryArena temp_arena = arena_alloc(Megabytes(1));
    if (!temp_arena.base) {
        core::printf("[Error] Failed to allocate temporary arena for shaders!\n");
        glXMakeCurrent(display, None, nullptr);
        glXDestroyContext(display, glx_context);
        return nullptr;
    }

    Path vs_path = path_parse(&temp_arena, str8_lit("assets/shaders/quads.vert"));
    Path fs_path = path_parse(&temp_arena, str8_lit("assets/shaders/quads.frag"));

    String8 vs_content = api.fs.read_entire_file(&temp_arena, vs_path);
    String8 fs_content = api.fs.read_entire_file(&temp_arena, fs_path);

    if (vs_content.size == 0 || fs_content.size == 0) {
        core::printf("[Error] Failed to load OpenGL shader files from disk!\n");
        arena_release(&temp_arena);
        glXMakeCurrent(display, None, nullptr);
        glXDestroyContext(display, glx_context);
        return nullptr;
    }

    // Dynamic #version header depending on query
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
        glXMakeCurrent(display, None, nullptr);
        glXDestroyContext(display, glx_context);
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

    // Create main texture id
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
    g_context.display = display;
    g_context.window = win;
    g_context.glx_context = glx_context;

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
    Display* display = (Display*)context->display;
    Window win = (Window)context->window;
    i32 width = *(i32*)((char*)window + sizeof(Display*) + sizeof(Window) + sizeof(Atom));
    i32 height = *(i32*)((char*)window + sizeof(Display*) + sizeof(Window) + sizeof(Atom) + sizeof(i32));
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
    glXSwapBuffers(display, win);
}

EXTERN_C void __opengl_destroy_graphics(GraphicsContext* context) noexcept {
    if (context) {
        Display* display = (Display*)context->display;
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
        if (context->glx_context) {
            glXMakeCurrent(display, None, nullptr);
            glXDestroyContext(display, (GLXContext)context->glx_context);
            context->glx_context = nullptr;
        }
    }
}
