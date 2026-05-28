#include "../platform/platform.h"

#if defined(OS_LINUX)

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

struct GraphicsContext {
    GLXContext glx_context;
};

// Global static instances for no-stdlib single-window architecture
static GraphicsContext g_context = {};

struct PlatformWindow {
    Display* display;
    Window window;
    Atom wm_delete_window;
    i32 width;
    i32 height;
};

static GraphicsContext* __linux_init_graphics(PlatformWindow* window) noexcept {
    if (!window || !window->display || !window->window) return nullptr;

    int visual_attribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo* vi = glXChooseVisual(window->display, DefaultScreen(window->display), visual_attribs);
    if (!vi) return nullptr;

    GLXContext glx_context = glXCreateContext(window->display, vi, nullptr, GL_TRUE);
    if (!glx_context) return nullptr;

    glXMakeCurrent(window->display, window->window, glx_context);

    g_context.glx_context = glx_context;
    return &g_context;
}

static void __linux_clear_screen(GraphicsContext* context, f32 r, f32 g, f32 b, f32 a) noexcept {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void __linux_swap_buffers(PlatformWindow* window, GraphicsContext* context) noexcept {
    if (window && window->display && window->window && context && context->glx_context) {
        glXSwapBuffers(window->display, window->window);
    }
}

static void __linux_destroy_graphics(GraphicsContext* context) noexcept {
    Display* active_display = glXGetCurrentDisplay();
    if (context && context->glx_context && active_display) {
        glXMakeCurrent(active_display, None, nullptr);
        glXDestroyContext(active_display, context->glx_context);
        context->glx_context = nullptr;
    }
}

static b8 __linux_upload_texture(GraphicsContext* context, const u8* pixels, i32 width, i32 height) noexcept {
    // Stub for Linux texture upload
    return true;
}

static void __linux_submit_frame(PlatformWindow* window, GraphicsContext* context, RenderCommandQueue queue) noexcept {
    if (!window || !context) return;
    
    // Process CPU render command queue (immediate buffer decoder)
    usize offset = 0;
    while (offset < queue.size) {
        RenderCommandHeader* header = (RenderCommandHeader*)(queue.buffer + offset);
        
        if (header->kind == RENDER_COMMAND_CLEAR) {
            RenderCommandClear* clear_cmd = (RenderCommandClear*)(queue.buffer + offset + sizeof(RenderCommandHeader));
            __linux_clear_screen(context, clear_cmd->r, clear_cmd->g, clear_cmd->b, clear_cmd->a);
        }
        // Quads not implemented on Linux yet
        
        offset += header->size;
    }
    
    __linux_swap_buffers(window, context);
}

#endif // OS_LINUX
