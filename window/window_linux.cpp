#include "../platform/platform.h"

#if defined(OS_LINUX)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

struct PlatformWindow {
    Display* display;
    Window window;
    Atom wm_delete_window;
    i32 width;
    i32 height;
};

// Global static instances for no-stdlib single-window architecture
static PlatformWindow g_window = {};

static PlatformWindow* __linux_create_window(const char* title, i32 width, i32 height) noexcept {
    core::printf("[System] X11 window init start (%dx%d).\n", width, height);
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return nullptr;
    }

    int visual_attribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo* vi = glXChooseVisual(display, DefaultScreen(display), visual_attribs);
    if (!vi) {
        XCloseDisplay(display);
        return nullptr;
    }

    core::printf("[System] X11 visual selected.\n");

    Colormap cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);

    XSetWindowAttributes swa = {};
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    Window window = XCreateWindow(
        display, RootWindow(display, vi->screen),
        0, 0, width, height, 0,
        vi->depth, InputOutput, vi->visual,
        CWColormap | CWEventMask, &swa
    );

    XStoreName(display, window, title);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XMapWindow(display, window);

    core::printf("[System] X11 window created and mapped.\n");

    g_window.display = display;
    g_window.window = window;
    g_window.wm_delete_window = wm_delete_window;
    g_window.width = width;
    g_window.height = height;

    return &g_window;
}

static void __linux_destroy_window(PlatformWindow* window) noexcept {
    if (window && window->display) {
        if (window->window) {
            XDestroyWindow(window->display, window->window);
            window->window = 0;
        }
        XCloseDisplay(window->display);
        window->display = nullptr;
    }
}

static b8 __linux_poll_events(PlatformWindow* window, i32* out_width, i32* out_height, b8* out_quit) noexcept {
    if (!window || !window->display || !window->window) return false;

    b8 got_events = false;
    while (XPending(window->display)) {
        XEvent event;
        XNextEvent(window->display, &event);
        got_events = true;

        switch (event.type) {
            case ConfigureNotify: {
                window->width = event.xconfigure.width;
                window->height = event.xconfigure.height;
                if (out_width) *out_width = window->width;
                if (out_height) *out_height = window->height;
            } break;

            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == window->wm_delete_window) {
                    if (out_quit) *out_quit = true;
                }
            } break;
        }
    }
    return got_events;
}

#endif // OS_LINUX
