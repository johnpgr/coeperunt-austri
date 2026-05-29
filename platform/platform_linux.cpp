#include "platform.h"

EXTERN_C char** environ = nullptr;

#if defined(__x86_64__)
__asm__(".global _start\n"
        "_start:\n"
        "movq (%rsp), %rdi\n"  // argc
        "leaq 8(%rsp), %rsi\n" // argv
        "movq %rdi, %rax\n"
        "shlq $3, %rax\n"            // argc * 8
        "leaq 16(%rsp,%rax), %rdx\n" // envp = &argv[argc + 1]
        "movq %rdx, environ(%rip)\n" // Store envp in global environ
        "call nomain_entry\n"
        "hlt\n");
#elif defined(__aarch64__)
__asm__(".global _start\n"
        "_start:\n"
        "ldr x0, [sp]\n"           // argc
        "add x1, sp, #8\n"         // argv = sp + 8
        "add x2, x0, #1\n"         // argc + 1
        "add x2, x1, x2, lsl #3\n" // envp = argv + (argc + 1) * 8
        "adrp x3, environ\n"
        "str x2, [x3, #:lo12:environ]\n"
        "bl nomain_entry\n");
#endif

// ---- 1-argument syscall (e.g. exit) ------------------------------------
static inline i64 __syscall1(i64 number, i64 a0) noexcept {
    i64 ret;
#if defined(__x86_64__)
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(number), "D"(a0)
                     : "rcx", "r11", "memory");
#elif defined(__aarch64__)
    register i64 x8 __asm__("x8") = number;
    register i64 x0 __asm__("x0") = a0;
    __asm__ volatile("svc #0" : "=r"(x0) : "r"(x8), "0"(x0) : "memory");
    ret = x0;
#else
#error "Unsupported Linux architecture"
#endif
    return ret;
}

// ---- 2-argument syscall (e.g. munmap) ----------------------------------
static inline i64 __syscall2(i64 number, i64 a0, i64 a1) noexcept {
    i64 ret;
#if defined(__x86_64__)
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(number), "D"(a0), "S"(a1)
                     : "rcx", "r11", "memory");
#elif defined(__aarch64__)
    register i64 x8 __asm__("x8") = number;
    register i64 x0 __asm__("x0") = a0;
    register i64 x1 __asm__("x1") = a1;
    __asm__ volatile("svc #0"
                     : "=r"(x0)
                     : "r"(x8), "0"(x0), "r"(x1)
                     : "memory");
    ret = x0;
#else
#error "Unsupported Linux architecture"
#endif
    return ret;
}

// ---- 3-argument syscall (e.g. read/write/lseek) ------------------------
static inline i64 __syscall3(i64 number, i64 a0, i64 a1, i64 a2) noexcept {
    i64 ret;
#if defined(__x86_64__)
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(number), "D"(a0), "S"(a1), "d"(a2)
                     : "rcx", "r11", "memory");
#elif defined(__aarch64__)
    register i64 x8 __asm__("x8") = number;
    register i64 x0 __asm__("x0") = a0;
    register i64 x1 __asm__("x1") = a1;
    register i64 x2 __asm__("x2") = a2;
    __asm__ volatile("svc #0"
                     : "=r"(x0)
                     : "r"(x8), "0"(x0), "r"(x1), "r"(x2)
                     : "memory");
    ret = x0;
#else
#error "Unsupported Linux architecture"
#endif
    return ret;
}

// ---- 4-argument syscall (e.g. openat) ----------------------------------
static inline i64 __syscall4(
    i64 number,
    i64 a0,
    i64 a1,
    i64 a2,
    i64 a3
) noexcept {
    i64 ret;
#if defined(__x86_64__)
    register i64 r10 __asm__("r10") = a3;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(number), "D"(a0), "S"(a1), "d"(a2), "r"(r10)
                     : "rcx", "r11", "memory");
#elif defined(__aarch64__)
    register i64 x8 __asm__("x8") = number;
    register i64 x0 __asm__("x0") = a0;
    register i64 x1 __asm__("x1") = a1;
    register i64 x2 __asm__("x2") = a2;
    register i64 x3 __asm__("x3") = a3;
    __asm__ volatile("svc #0"
                     : "=r"(x0)
                     : "r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3)
                     : "memory");
    ret = x0;
#else
#error "Unsupported Linux architecture"
#endif
    return ret;
}

// ---- 6-argument syscall (e.g. mmap) ------------------------------------
static inline i64 __syscall6(
    i64 number,
    i64 a0,
    i64 a1,
    i64 a2,
    i64 a3,
    i64 a4,
    i64 a5
) noexcept {
    i64 ret;
#if defined(__x86_64__)
    register i64 r10 __asm__("r10") = a3;
    register i64 r8 __asm__("r8")   = a4;
    register i64 r9 __asm__("r9")   = a5;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(number), "D"(a0), "S"(a1), "d"(a2), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
#elif defined(__aarch64__)
    register i64 x8 __asm__("x8") = number;
    register i64 x0 __asm__("x0") = a0;
    register i64 x1 __asm__("x1") = a1;
    register i64 x2 __asm__("x2") = a2;
    register i64 x3 __asm__("x3") = a3;
    register i64 x4 __asm__("x4") = a4;
    register i64 x5 __asm__("x5") = a5;
    __asm__ volatile(
        "svc #0"
        : "=r"(x0)
        : "r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
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
static const i64 SYS_READ   = 0;
static const i64 SYS_WRITE  = 1;
static const i64 SYS_OPENAT = 257;
static const i64 SYS_CLOSE  = 3;
static const i64 SYS_LSEEK  = 8;
static const i64 SYS_MMAP   = 9;
static const i64 SYS_MUNMAP = 11;
static const i64 SYS_EXIT   = 60;
#elif defined(__aarch64__)
static const i64 SYS_READ   = 63;
static const i64 SYS_WRITE  = 64;
static const i64 SYS_OPENAT = 56;
static const i64 SYS_CLOSE  = 57;
static const i64 SYS_LSEEK  = 62;
static const i64 SYS_MMAP   = 222;
static const i64 SYS_MUNMAP = 215;
static const i64 SYS_EXIT   = 93;
#endif

static const i64 LINUX_O_RDONLY = 0;
static const i64 LINUX_O_WRONLY = 1;
static const i64 LINUX_O_RDWR   = 2;
static const i64 LINUX_O_CREAT  = 0100;
static const i64 LINUX_O_TRUNC  = 01000;
static const i64 LINUX_AT_FDCWD = -100;

static const i64 LINUX_SEEK_SET = 0;
static const i64 LINUX_SEEK_END = 2;

static const i64 LINUX_PROT_READ     = 0x1;
static const i64 LINUX_PROT_WRITE    = 0x2;
static const i64 LINUX_MAP_PRIVATE   = 0x02;
static const i64 LINUX_MAP_ANONYMOUS = 0x20;

#define LINUX_MAP_FAILED ((void*)-1)

// ============================================================================
// Global functions implementation
// ============================================================================
namespace core {

NORETURN void exit(i32 status) noexcept {
    __syscall1(SYS_EXIT, status);
    while (1) {
    } // Guarantee termination
}

void print(const char* message) noexcept {
    if (!message)
        return;
    usize len = core::strlen(message);
    __syscall3(SYS_WRITE, 2, (i64)message, (i64)len);
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

} // namespace core

// Physically separate implementations included in Unity Build order
#include "../core/memory/arena_linux.cpp"
#include "fs/fs_linux.cpp"
#include "media/media_linux.cpp"
#include "window/window_linux.cpp"
#include "../render/opengl/opengl_renderer_linux.cpp"

void platform_init(PlatformApi* api) noexcept {
    if (api) {
        api->window.create  = __linux_create_window;
        api->window.destroy = __linux_destroy_window;
        api->window.poll    = __linux_poll_events;

        api->render.init           = __opengl_init_graphics;
        api->render.upload_texture = __opengl_upload_texture;
        api->render.submit_frame   = __opengl_submit_frame;
        api->render.destroy        = __opengl_destroy_graphics;

        api->fs.read_entire_file  = linux_read_entire_file;
        api->fs.write_entire_file = linux_write_entire_file;

        api->media.load_bmp = linux_load_bmp;
        api->media.free_bmp = linux_free_bmp;
    }
}
