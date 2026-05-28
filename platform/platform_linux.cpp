#include "platform.h"

#if defined(OS_LINUX)

#if defined(__x86_64__)
__asm__(
    ".global _start\n"
    "_start:\n"
    "movq (%rsp), %rdi\n"      // argc
    "leaq 8(%rsp), %rsi\n"     // argv
    "movq %rdi, %rax\n"
    "shlq $3, %rax\n"          // argc * 8
    "leaq 16(%rsp,%rax), %rdx\n" // envp = &argv[argc + 1]
    "call nomain_entry\n"
    "hlt\n"
);
#elif defined(__aarch64__)
__asm__(
    ".global _start\n"
    "_start:\n"
    "ldr x0, [sp]\n"           // argc
    "add x1, sp, #8\n"         // argv = sp + 8
    "add x2, x0, #1\n"         // argc + 1
    "add x2, x1, x2, lsl #3\n" // envp = argv + (argc + 1) * 8
    "bl nomain_entry\n"
);
#endif

// ---- 2-argument syscall (e.g. exit) ------------------------------------
static inline i64 __syscall2(i64 number, i64 a0) noexcept {
    i64 ret;
    #if defined(__x86_64__)
        __asm__ volatile (
            "syscall"
            : "=a"(ret)
            : "a"(number), "D"(a0)
            : "rcx", "r11", "memory"
        );
    #elif defined(__aarch64__)
        register i64 x8 __asm__("x8") = number;
        register i64 x0 __asm__("x0") = a0;
        __asm__ volatile (
            "svc #0"
            : "=r"(x0)
            : "r"(x8), "0"(x0)
            : "memory"
        );
        ret = x0;
    #else
        #error "Unsupported Linux architecture"
    #endif
    return ret;
}

// ---- 4-argument syscall (e.g. write) -----------------------------------
static inline i64 __syscall4(i64 number, i64 a0, i64 a1, i64 a2) noexcept {
    i64 ret;
    #if defined(__x86_64__)
        __asm__ volatile (
            "syscall"
            : "=a"(ret)
            : "a"(number), "D"(a0), "S"(a1), "d"(a2)
            : "rcx", "r11", "memory"
        );
    #elif defined(__aarch64__)
        register i64 x8 __asm__("x8") = number;
        register i64 x0 __asm__("x0") = a0;
        register i64 x1 __asm__("x1") = a1;
        register i64 x2 __asm__("x2") = a2;
        __asm__ volatile (
            "svc #0"
            : "=r"(x0)
            : "r"(x8), "0"(x0), "r"(x1), "r"(x2)
            : "memory"
        );
        ret = x0;
    #else
        #error "Unsupported Linux architecture"
    #endif
    return ret;
}

// Syscall numbers (C-compatible const values)
#if defined(__x86_64__)
    static const i64 SYS_WRITE = 1;
    static const i64 SYS_EXIT  = 60;
#elif defined(__aarch64__)
    static const i64 SYS_WRITE = 64;
    static const i64 SYS_EXIT  = 93;
#endif

// ============================================================================
// Global functions implementation
// ============================================================================
namespace core {
    NORETURN void exit(i32 status) noexcept {
        __syscall2(SYS_EXIT, status);
        while (1) {} // Guarantee termination
    }

    void print(const char* message) noexcept {
        if (!message) return;
        usize len = core::strlen(message);
        __syscall4(SYS_WRITE, 2, (i64)message, (i64)len);
    }

    void printv(const char* format, va_list args) noexcept {
        char buf[16384];
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
}

// ============================================================================
// File System Stubs (Linux)
// ============================================================================
static FileContent __linux_read_entire_file(const char* filepath) noexcept {
    FileContent result = {};
    return result;
}

static b8 __linux_write_entire_file(const char* filepath, const void* data, usize size) noexcept {
    return false;
}

static void __linux_free_file_content(FileContent content) noexcept {}

// ============================================================================
// Media Stubs (Linux)
// ============================================================================
static LoadedImage __linux_load_bmp(const char* filepath) noexcept {
    LoadedImage result = {};
    return result;
}

static void __linux_free_bmp(LoadedImage image) noexcept {}

// Physically separate implementations included in Unity Build order
#include "../window/window_linux.cpp"
#include "../render/render_linux.cpp"

void platform_init(PlatformApi* api) noexcept {
    if (api) {
        api->window.create  = __linux_create_window;
        api->window.destroy = __linux_destroy_window;
        api->window.poll    = __linux_poll_events;

        api->render.init           = __linux_init_graphics;
        api->render.upload_texture = __linux_upload_texture;
        api->render.submit_frame   = __linux_submit_frame;
        api->render.destroy        = __linux_destroy_graphics;

        api->fs.read_entire_file  = __linux_read_entire_file;
        api->fs.write_entire_file = __linux_write_entire_file;
        api->fs.free_file_content = __linux_free_file_content;

        api->media.load_bmp = __linux_load_bmp;
        api->media.free_bmp = __linux_free_bmp;
    }
}

#endif // OS_LINUX
