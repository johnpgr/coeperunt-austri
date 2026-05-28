#include "basic.h"

#if defined(OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Custom symbols for no-stdlib environment
extern "C" {
    int _fltused = 0;
}

static void __win32_debug_log(const char* message) NOEXCEPT {
    if (message) {
        OutputDebugStringA(message);
    }
}

static void __win32_debug_logv(const char* format, va_list args) NOEXCEPT {
    char buf[2048];
    va_list args_copy;
    va_copy(args_copy, args);
    custom_vsnprintf(buf, sizeof(buf), format, args_copy);
    va_end(args_copy);
    __win32_debug_log(buf);
}

static void __win32_debug_logf(const char* format, ...) NOEXCEPT {
    va_list args;
    va_start(args, format);
    __win32_debug_logv(format, args);
    va_end(args);
}

NORETURN static void __win32_exit(i32 status) NOEXCEPT {
    ExitProcess((u32)status);
    while (1) {} // Guarantee termination
}

void basic_init(Basic* basic) NOEXCEPT {
    if (basic) {
        basic->print  = __win32_debug_log;
        basic->printf = __win32_debug_logf;
        basic->printv = __win32_debug_logv;
        basic->exit   = __win32_exit;
    }
}

#endif // OS_WINDOWS
