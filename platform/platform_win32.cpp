#include "platform.h"

#define PLATFORM_WIN32_IMPLEMENTATION
#include "win32.h"

// Custom symbols for no-stdlib environment
EXTERN_C int _fltused = 0;

extern "C" void win32_load_dynamic_symbols() noexcept {
    if (win32::AdjustWindowRect) return; // Already loaded

    HMODULE user32 = LoadLibraryA("user32.dll");
    if (user32) {
        PLATFORM_LOAD(user32, AdjustWindowRect);
        PLATFORM_LOAD(user32, CreateWindowExA);
        PLATFORM_LOAD(user32, DestroyWindow);
        PLATFORM_LOAD(user32, DefWindowProcA);
        PLATFORM_LOAD(user32, DispatchMessageA);
        PLATFORM_LOAD(user32, LoadCursorA);
        PLATFORM_LOAD(user32, PeekMessageA);
        PLATFORM_LOAD(user32, TranslateMessage);
        PLATFORM_LOAD(user32, PostQuitMessage);
        PLATFORM_LOAD(user32, RegisterClassExA);
        PLATFORM_LOAD(user32, ShowWindow);
        PLATFORM_LOAD(user32, UpdateWindow);
        PLATFORM_LOAD(user32, SetCapture);
        PLATFORM_LOAD(user32, ReleaseCapture);
    }
}

// ============================================================================
// Global functions implementation
// ============================================================================
namespace core {

NORETURN 
void exit(i32 status) noexcept {
    ExitProcess((u32)status);
    while (1) {} // Guarantee termination
}

void print(const char* message) noexcept {
    if (message) {
        OutputDebugStringA(message);
    }
}

void printv(const char* format, va_list args) noexcept {
    char buf[2048];
    va_list args_copy;
    va_copy(args_copy, args);
    core::vsnprintf(buf, sizeof(buf), format, args_copy);
    va_end(args_copy);
    core::print(buf);
}

void printf(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    core::printv(format, args);
    va_end(args);
}

} // namespace core

// Physically separate implementations included in Unity Build order
#include "../core/memory/arena_win32.cpp"
#include "fs/fs_win32.cpp"
#include "media/media_win32.cpp"
#include "window/window_win32.cpp"
#include "../render/opengl/opengl_renderer_win32.cpp"

void platform_init(PlatformApi* api) noexcept {
    win32_load_dynamic_symbols();

    if (!api) {
        core::printf("[Error] Invalid platform API pointer\n");
        return;
    }

    api->window.create  = __win32_create_window;
    api->window.destroy = __win32_destroy_window;
    api->window.poll    = __win32_poll_events;

    api->render.init           = __opengl_init_graphics;
    api->render.upload_texture = __opengl_upload_texture;
    api->render.submit_frame   = __opengl_submit_frame;
    api->render.destroy        = __opengl_destroy_graphics;

    api->fs.read_entire_file  = win32_read_entire_file;
    api->fs.write_entire_file = win32_write_entire_file;

    api->media.load_bmp = win32_load_bmp;
    api->media.free_bmp = win32_free_bmp;
}
