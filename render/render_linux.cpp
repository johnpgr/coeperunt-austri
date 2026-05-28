#include "render.h"

#if defined(OS_LINUX)

#include <GL/gl.h>
#include <GL/glx.h>

struct GraphicsContext {
    GLXContext glx_context;
};

// Global static instances for no-stdlib single-window architecture
static GraphicsContext g_context = {};

static GraphicsContext* __linux_init_graphics(PlatformWindow* window) NOEXCEPT {
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

static void __linux_clear_screen(GraphicsContext* context, f32 r, f32 g, f32 b, f32 a) NOEXCEPT {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void __linux_swap_buffers(PlatformWindow* window, GraphicsContext* context) NOEXCEPT {
    if (window && window->display && window->window && context && context->glx_context) {
        glXSwapBuffers(window->display, window->window);
    }
}

static void __linux_destroy_graphics(GraphicsContext* context) NOEXCEPT {
    Display* active_display = glXGetCurrentDisplay();
    if (context && context->glx_context && active_display) {
        glXMakeCurrent(active_display, None, nullptr);
        glXDestroyContext(active_display, context->glx_context);
        context->glx_context = nullptr;
    }
}

void render_init(RenderApi* render) NOEXCEPT {
    if (render) {
        render->init    = __linux_init_graphics;
        render->clear   = __linux_clear_screen;
        render->swap    = __linux_swap_buffers;
        render->destroy = __linux_destroy_graphics;
    }
}

#endif // OS_LINUX
