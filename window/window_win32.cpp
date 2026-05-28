#include "../platform/platform.h"

#if defined(OS_WINDOWS)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
// TODO(maybe): Remove windows.h import and load the win32 library functions dynamically
#include <windows.h>

struct PlatformWindow {
    HWND hwnd;
    i32 width;
    i32 height;
    b8 should_close;
};

// Global static instances for no-stdlib single-window architecture
static PlatformWindow g_window = {};

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
        case WM_DESTROY: {
            g_window.should_close = true;
            PostQuitMessage(0);
            return 0;
        } break;
        
        case WM_SIZE: {
            g_window.width = LOWORD(lParam);
            g_window.height = HIWORD(lParam);
            return 0;
        } break;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static PlatformWindow* __win32_create_window(const char* title, i32 width, i32 height) noexcept {
    core::printf("[System] Win32 window init start (%dx%d).\n", width, height);
    HINSTANCE hinstance = GetModuleHandleA(nullptr);
    
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursorA(nullptr, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "UnnamedGameWindowClass";
    
    if (!RegisterClassExA(&wc)) {
        return nullptr;
    }

    core::printf("[System] Win32 window class registered.\n");
    
    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr,
        hinstance, nullptr
    );
    
    if (!hwnd) {
        return nullptr;
    }

    core::printf("[System] Win32 window created.\n");
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    core::printf("[System] Win32 window shown.\n");
    
    g_window.hwnd = hwnd;
    g_window.width = width;
    g_window.height = height;
    g_window.should_close = false;
    
    return &g_window;
}

static void __win32_destroy_window(PlatformWindow* window) noexcept {
    if (window && window->hwnd) {
        DestroyWindow(window->hwnd);
        window->hwnd = nullptr;
    }
}

static b8 __win32_poll_events(PlatformWindow* window, i32* out_width, i32* out_height, b8* out_quit) noexcept {
    if (!window || !window->hwnd) return false;
    
    MSG msg = {};
    b8 got_events = false;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        got_events = true;
    }
    
    if (out_width) *out_width = window->width;
    if (out_height) *out_height = window->height;
    if (out_quit) *out_quit = window->should_close;
    
    return got_events;
}

#endif // OS_WINDOWS
