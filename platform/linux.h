#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#include "../core.h"

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

namespace sys {

inline void exit(i32 status) noexcept {
    __syscall1(SYS_EXIT, status);
}

inline i64 write(i64 fd, const void* buf, usize count) noexcept {
    return __syscall3(SYS_WRITE, fd, (i64)buf, (i64)count);
}

inline i64 read(i64 fd, void* buf, usize count) noexcept {
    return __syscall3(SYS_READ, fd, (i64)buf, (i64)count);
}

inline i64 openat(i64 dirfd, const char* pathname, i64 flags, i64 mode) noexcept {
    return __syscall4(SYS_OPENAT, dirfd, (i64)pathname, flags, mode);
}

inline i64 close(i64 fd) noexcept {
    return __syscall1(SYS_CLOSE, fd);
}

inline i64 lseek(i64 fd, i64 offset, i32 whence) noexcept {
    return __syscall3(SYS_LSEEK, fd, offset, (i64)whence);
}

inline void* mmap(void* addr, usize length, i32 prot, i32 flags, i32 fd, i64 offset) noexcept {
    i64 res = __syscall6(SYS_MMAP, (i64)addr, (i64)length, (i64)prot, (i64)flags, (i64)fd, offset);
    return (void*)res;
}

inline i32 munmap(void* addr, usize length) noexcept {
    return (i32)__syscall2(SYS_MUNMAP, (i64)addr, (i64)length);
}

} // namespace linux_sys

#endif // PLATFORM_LINUX_H
