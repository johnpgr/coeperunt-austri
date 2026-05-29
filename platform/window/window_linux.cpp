#include "../platform.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "../../render/opengl/glx/glx_linux.h"

struct PlatformWindow {
    Display* display;
    Window window;
    Atom wm_delete_window;
    i32 width;
    i32 height;
    i32 mouse_x;
    i32 mouse_y;
    b8 mouse_left_down;
    b8 mouse_right_down;
    b8 should_close;
};

// Global static instances for no-stdlib single-window architecture
static PlatformWindow g_window = {};

static PlatformWindow* __linux_create_window(const char* title, i32 width, i32 height) noexcept {
    core::printf("[System] X11 window init start (%dx%d).\n", width, height);

    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return nullptr;
    }

    if (!glx_load_symbols()) {
        XCloseDisplay(display);
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
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

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
    g_window.mouse_x = 0;
    g_window.mouse_y = 0;
    g_window.mouse_left_down = false;
    g_window.mouse_right_down = false;
    g_window.should_close = false;

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
    glx_unload_symbols();
}

static b8 __linux_poll_events(PlatformWindow* window) noexcept {
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
            } break;

            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == window->wm_delete_window) {
                    window->should_close = true;
                }
            } break;

            case MotionNotify: {
                window->mouse_x = event.xmotion.x;
                window->mouse_y = event.xmotion.y;
            } break;

            case ButtonPress: {
                if (event.xbutton.button == Button1) {
                    window->mouse_left_down = true;
                } else if (event.xbutton.button == Button3) {
                    window->mouse_right_down = true;
                }
            } break;

            case ButtonRelease: {
                if (event.xbutton.button == Button1) {
                    window->mouse_left_down = false;
                } else if (event.xbutton.button == Button3) {
                    window->mouse_right_down = false;
                }
            } break;
        }
    }
    
    api.window.width = window->width;
    api.window.height = window->height;
    api.quit = window->should_close;
    api.input.mouse.x = window->mouse_x;
    api.input.mouse.y = window->mouse_y;
    api.input.mouse.left_down = window->mouse_left_down;
    api.input.mouse.right_down = window->mouse_right_down;
    
    return got_events;
}
