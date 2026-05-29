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

#include "linux.h"

// ============================================================================
// Global functions implementation
// ============================================================================
namespace core {

NORETURN void exit(i32 status) noexcept {
    sys::exit(status);
    while (1) {} // Guarantee termination
}

void print(const char* message) noexcept {
    if (!message)
        return;
    usize len = core::strlen(message);
    sys::write(2, message, len);
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
#include "../render/opengl/glx/glx_linux.cpp"
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
