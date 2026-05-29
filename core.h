#ifndef CORE_H
#define CORE_H

// ============================================================================
// Compile-Time OS Detection & Linking Directives
// ============================================================================
#if defined(_WIN32)
    #define OS_WINDOWS
#elif defined(__linux__)
    #define OS_LINUX
#else
    #error "Unsupported Operating System"
#endif

#define EXTERN_C extern "C"
#if defined(_MSC_VER) && !defined(__clang__)
    #define NORETURN __declspec(noreturn)
#elif defined(__clang__) || defined(__GNUC__)
    #define NORETURN __attribute__((noreturn))
#else
    #define NORETURN [[noreturn]]
#endif

typedef signed char        i8;
typedef unsigned char      u8;
typedef short              i16;
typedef unsigned short     u16;
typedef int                i32;
typedef unsigned int       u32;
typedef long long          i64;
typedef unsigned long long u64;
typedef float              f32;
typedef double             f64;
typedef __SIZE_TYPE__      usize;
typedef __PTRDIFF_TYPE__   isize;
typedef bool               b8;
typedef int                b32;

#define Kilobytes(val) ((val)*1024ULL)
#define Megabytes(val) (Kilobytes(val)*1024ULL)
#define Gigabytes(val) (Megabytes(val)*1024ULL)
#define Terabytes(val) (Gigabytes(val)*1024ULL)

#define internal        static
#define local_persist   static
#define global          static

#if defined(OS_WINDOWS)
#define CORE_PRINTV_BUFFER_SIZE 2048
#else
#define CORE_PRINTV_BUFFER_SIZE 16384
#endif

#define CORE_LIT_LEN(s) ((usize)(sizeof(s) - 1))
#define CORE_PRINT_LIT(s) core::print_raw((s), CORE_LIT_LEN(s))
#define CORE_PRINTV_LIT(fmt, args) core::printv_len((fmt), CORE_LIT_LEN(fmt), (args))
#define CORE_HAS_ARGS(...) CORE_HAS_ARGS_IMPL(__VA_OPT__(,) 1, 0)
#define CORE_HAS_ARGS_IMPL(_0, _1, ...) _1
#define CORE_PRINTF_LIT_0(fmt, ...) printf_lit0((fmt), CORE_LIT_LEN(fmt))
#define CORE_PRINTF_LIT_1(fmt, ...) printf_litv((fmt), CORE_LIT_LEN(fmt), __VA_ARGS__)
#define CORE_PRINTF_LIT_DISPATCH(has_args, fmt, ...) CORE_PRINTF_LIT_DISPATCH2(has_args, fmt, __VA_ARGS__)
#define CORE_PRINTF_LIT_DISPATCH2(has_args, fmt, ...) CORE_PRINTF_LIT_##has_args(fmt, __VA_ARGS__)
#define CORE_PRINTF_LIT(fmt, ...) CORE_PRINTF_LIT_DISPATCH(CORE_HAS_ARGS(__VA_ARGS__), fmt __VA_OPT__(,) __VA_ARGS__)
#define printf(...) CORE_PRINTF_LIT(__VA_ARGS__)

#if DEBUG
#define assert(expression) if(!(expression)) { *(volatile int *)0 = 0; }
#else
#define assert(expression)
#endif

// ============================================================================
// Custom String Utilities, Trigonometrics, Memory & IO (Freestanding core namespace)
// ============================================================================

namespace core {
    inline constexpr usize strlen(const char* str) {
        usize len = 0;
        while (str && str[len] != '\0') {
            len++;
        }
        return len;
    }

    // ============================================================================
    // Freestanding Trigonometric Helpers
    // ============================================================================
    #define PI 3.14159265358979323846f
    #define TWO_PI 6.28318530717958647692f
    #define HALF_PI 1.57079632679489661923f

    static inline f32 sin(f32 x) {
        // 1. Range reduction to [-PI, PI]
        while (x > PI)  x -= TWO_PI;
        while (x < -PI) x += TWO_PI;
        
        // 2. Taylor series approximation: x - x^3/6 + x^5/120 - x^7/5040 + x^9/362880
        f32 x2 = x * x;
        f32 x3 = x * x2;
        f32 x5 = x3 * x2;
        f32 x7 = x5 * x2;
        f32 x9 = x7 * x2;
        
        return x - (x3 * 0.166666666667f) + (x5 * 0.008333333333f) - (x7 * 0.000198412698f) + (x9 * 0.000002755731f);
    }

    static inline f32 cos(f32 x) {
        return sin(x + HALF_PI);
    }
}

// ============================================================================
// Unified Entry Point
// ============================================================================
#if defined(OS_WINDOWS)
    #define MAIN EXTERN_C void no_crt_entry()
#elif defined(OS_LINUX)
    #define MAIN EXTERN_C void nomain_entry(int argc, char** argv, char** envp)
#endif

// ============================================================================
// Freestanding CRT Overrides (No-stdlib compiler safeties at global scope)
// ============================================================================
EXTERN_C {
    void* memset(void* dest, int c, usize size);
    void* memcpy(void* dest, const void* src, usize size);
}

#if defined(OS_WINDOWS)
EXTERN_C {
    #if defined(_MSC_VER) && !defined(__clang__)
        #pragma function(memset)
    #endif
    void* memset(void* dest, int c, usize size) {
        unsigned char* p = (unsigned char*)dest;
        while (size--) *p++ = (unsigned char)c;
        return dest;
    }

    #if defined(_MSC_VER) && !defined(__clang__)
        #pragma function(memcpy)
    #endif
    void* memcpy(void* dest, const void* src, usize size) {
        unsigned char* d = (unsigned char*)dest;
        const unsigned char* s = (const unsigned char*)src;
        while (size--) *d++ = *s++;
        return dest;
    }
}
#endif

// ============================================================================
// va_list macros
// ============================================================================
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
typedef char* va_list;
extern "C" void __va_start(va_list*, ...);
#define va_start(ap, v) __va_start(&ap, &(v), ((sizeof(v) + sizeof(long long) - 1) & ~(sizeof(long long) - 1)), __alignof(v), &(v))
#define va_arg(ap, t)   (*(t*)((ap += ((sizeof(t) + sizeof(long long) - 1) & ~(sizeof(long long) - 1))) - ((sizeof(t) + sizeof(long long) - 1) & ~(sizeof(long long) - 1))))
#define va_end(ap)      ((void)(ap = (va_list)0))
#define va_copy(dest, src) ((void)((dest) = (src)))
#else
#ifndef va_list
typedef __builtin_va_list va_list;
#endif
#ifndef va_start
#define va_start(ap, last) __builtin_va_start(ap, last)
#endif
#ifndef va_arg
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#endif
#ifndef va_copy
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#endif
#ifndef va_end
#define va_end(ap) __builtin_va_end(ap)
#endif
#endif

// ============================================================================
// Namespace core optimized memory operations & IO declarations
// ============================================================================
namespace core {
    inline void* memset(void* dest, int c, usize size) {
#if defined(__clang__) || defined(__GNUC__)
        return __builtin_memset(dest, c, size);
#else
        return ::memset(dest, c, size);
#endif
    }

    inline void* memcpy(void* dest, const void* src, usize size) {
#if defined(__clang__) || defined(__GNUC__)
        return __builtin_memcpy(dest, src, size);
#else
        return ::memcpy(dest, src, size);
#endif
    }

    // IO / System Functions implemented in platform layer
    void print_raw(const char* message, usize len) noexcept;

    void printv(const char* format, va_list args) noexcept;
    i32 vsnprintf_len(char* buf, usize buf_size, const char* format, usize format_len, va_list args) noexcept;

    inline void printv_len(const char* format, usize len, va_list args) noexcept {
        if (!format || len == 0) return;
        char buf[CORE_PRINTV_BUFFER_SIZE];
        va_list args_copy;
        va_copy(args_copy, args);
        i32 written = core::vsnprintf_len(buf, sizeof(buf), format, len, args_copy);
        va_end(args_copy);
        if (written > 0) {
            core::print_raw(buf, (usize)written);
        }
    }

    void printf_ptr(const char* format, ...) noexcept;

    inline void printf_lit0(const char* format, usize len) noexcept {
        if (!format || len == 0) return;
        core::print_raw(format, len);
    }

    inline void printf_litv(const char* format, usize len, ...) noexcept {
        if (!format || len == 0) return;
        va_list args;
        va_start(args, len);
        core::printv_len(format, len, args);
        va_end(args);
    }

    NORETURN void exit(i32 status) noexcept;

    // ============================================================================
    // vsnprintf Formatter Helper (using core::memset/memcpy)
    // ============================================================================
    inline i32 vsnprintf(char* buf, usize buf_size, const char* format, va_list args) noexcept {
        if (!buf || buf_size == 0) return 0;
        
        char* p = buf;
        char* end = buf + buf_size;
        
        while (p < end - 1 && *format) {
            if (*format == '%' && *(format + 1) != '\0') {
                format++; // skip '%'
                char spec = *format++;
                
                if (spec == '%') {
                    *p++ = '%';
                    continue;
                }
                
                if (spec == 's') {
                    const char* val = va_arg(args, const char*);
                    if (!val) val = "(null)";
                    usize val_len = core::strlen(val);
                    usize available = (usize)(end - 1 - p);
                    usize to_copy = (val_len < available) ? val_len : available;
                    if (to_copy > 0) {
                        core::memcpy(p, val, to_copy);
                        p += to_copy;
                    }
                } else if (spec == 'd' || spec == 'i') {
                    i64 val = va_arg(args, int);
                    char tmp[32];
                    i32 i = 0;
                    b8 neg = false;
                    if (val < 0) {
                        neg = true;
                        u64 uval = (u64)-val;
                        do {
                            tmp[i++] = '0' + (uval % 10);
                            uval /= 10;
                        } while (uval > 0 && i < 32);
                    } else {
                        u64 uval = (u64)val;
                        do {
                            tmp[i++] = '0' + (uval % 10);
                            uval /= 10;
                        } while (uval > 0 && i < 32);
                    }
                    if (neg && p < end - 1) {
                        *p++ = '-';
                    }
                    while (i > 0 && p < end - 1) {
                        *p++ = tmp[--i];
                    }
                } else if (spec == 'u') {
                    u64 val = va_arg(args, unsigned int);
                    char tmp[32];
                    i32 i = 0;
                    do {
                        tmp[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0 && i < 32);
                    while (i > 0 && p < end - 1) {
                        *p++ = tmp[--i];
                    }
                } else if (spec == 'x' || spec == 'p') {
                    u64 val;
                    if (spec == 'p') {
                        val = (u64)(usize)va_arg(args, void*);
                    } else {
                        val = va_arg(args, unsigned int);
                    }
                    char tmp[32];
                    i32 i = 0;
                    const char* digits = "0123456789abcdef";
                    do {
                        tmp[i++] = digits[val % 16];
                        val /= 16;
                    } while (val > 0 && i < 32);
                    while (i > 0 && p < end - 1) {
                        *p++ = tmp[--i];
                    }
                } else if (spec == 'c') {
                    char val = (char)va_arg(args, int);
                    *p++ = val;
                } else if (spec == 'f') {
                    f64 val = va_arg(args, double);
                    if (val < 0) {
                        if (p < end - 1) *p++ = '-';
                        val = -val;
                    }
                    u64 ipart = (u64)val;
                    
                    char tmp_i[32];
                    core::memset(tmp_i, 0, sizeof(tmp_i));
                    i32 idx = 0;
                    u64 ip = ipart;
                    do {
                        tmp_i[idx++] = '0' + (ip % 10);
                        ip /= 10;
                    } while (ip > 0 && idx < 32);
                    while (idx > 0 && p < end - 1) {
                        *p++ = tmp_i[--idx];
                    }
                    
                    if (p < end - 1) {
                        *p++ = '.';
                    }
                    
                    f64 fpart = val - (f64)ipart;
                    u64 fpart_val = (u64)(fpart * 1000000.0 + 0.5);
                    char tmp[6];
                    core::memset(tmp, '0', sizeof(tmp));
                    for (i32 i = 5; i >= 0; --i) {
                        tmp[i] = '0' + (fpart_val % 10);
                        fpart_val /= 10;
                    }
                    for (i32 i = 0; i < 6; ++i) {
                        if (p < end - 1) {
                            *p++ = tmp[i];
                        }
                    }
                } else {
                    if (p < end - 2) {
                        *p++ = '%';
                        *p++ = spec;
                    }
                }
            } else {
                *p++ = *format++;
            }
        }
        *p = '\0';
        return (i32)(p - buf);
    }

    inline i32 vsnprintf_len(char* buf, usize buf_size, const char* format, usize format_len, va_list args) noexcept {
        if (!buf || buf_size == 0 || !format || format_len == 0) return 0;

        char* p = buf;
        char* end = buf + buf_size;
        const char* fmt = format;
        const char* fmt_end = format + format_len;

        while (p < end - 1 && fmt < fmt_end) {
            if (*fmt == '%' && (fmt + 1) < fmt_end) {
                fmt++;
                char spec = *fmt++;

                if (spec == '%') {
                    *p++ = '%';
                    continue;
                }

                if (spec == 's') {
                    const char* val = va_arg(args, const char*);
                    if (!val) val = "(null)";
                    usize val_len = core::strlen(val);
                    usize available = (usize)(end - 1 - p);
                    usize to_copy = (val_len < available) ? val_len : available;
                    if (to_copy > 0) {
                        core::memcpy(p, val, to_copy);
                        p += to_copy;
                    }
                } else if (spec == 'd' || spec == 'i') {
                    i64 val = va_arg(args, int);
                    char tmp[32];
                    i32 i = 0;
                    b8 neg = false;
                    if (val < 0) {
                        neg = true;
                        u64 uval = (u64)-val;
                        do {
                            tmp[i++] = '0' + (uval % 10);
                            uval /= 10;
                        } while (uval > 0 && i < 32);
                    } else {
                        u64 uval = (u64)val;
                        do {
                            tmp[i++] = '0' + (uval % 10);
                            uval /= 10;
                        } while (uval > 0 && i < 32);
                    }
                    if (neg && p < end - 1) {
                        *p++ = '-';
                    }
                    while (i > 0 && p < end - 1) {
                        *p++ = tmp[--i];
                    }
                } else if (spec == 'u') {
                    u64 val = va_arg(args, unsigned int);
                    char tmp[32];
                    i32 i = 0;
                    do {
                        tmp[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0 && i < 32);
                    while (i > 0 && p < end - 1) {
                        *p++ = tmp[--i];
                    }
                } else if (spec == 'x' || spec == 'p') {
                    u64 val;
                    if (spec == 'p') {
                        val = (u64)(usize)va_arg(args, void*);
                    } else {
                        val = va_arg(args, unsigned int);
                    }
                    char tmp[32];
                    i32 i = 0;
                    const char* digits = "0123456789abcdef";
                    do {
                        tmp[i++] = digits[val % 16];
                        val /= 16;
                    } while (val > 0 && i < 32);
                    while (i > 0 && p < end - 1) {
                        *p++ = tmp[--i];
                    }
                } else if (spec == 'c') {
                    char val = (char)va_arg(args, int);
                    *p++ = val;
                } else if (spec == 'f') {
                    f64 val = va_arg(args, double);
                    if (val < 0) {
                        if (p < end - 1) *p++ = '-';
                        val = -val;
                    }
                    u64 ipart = (u64)val;

                    char tmp_i[32];
                    core::memset(tmp_i, 0, sizeof(tmp_i));
                    i32 idx = 0;
                    u64 ip = ipart;
                    do {
                        tmp_i[idx++] = '0' + (ip % 10);
                        ip /= 10;
                    } while (ip > 0 && idx < 32);
                    while (idx > 0 && p < end - 1) {
                        *p++ = tmp_i[--idx];
                    }

                    if (p < end - 1) {
                        *p++ = '.';
                    }

                    f64 fpart = val - (f64)ipart;
                    u64 fpart_val = (u64)(fpart * 1000000.0 + 0.5);
                    char tmp[6];
                    core::memset(tmp, '0', sizeof(tmp));
                    for (i32 i = 5; i >= 0; --i) {
                        tmp[i] = '0' + (fpart_val % 10);
                        fpart_val /= 10;
                    }
                    for (i32 i = 0; i < 6; ++i) {
                        if (p < end - 1) {
                            *p++ = tmp[i];
                        }
                    }
                } else {
                    if (p < end - 2) {
                        *p++ = '%';
                        *p++ = spec;
                    }
                }
            } else {
                *p++ = *fmt++;
            }
        }
        *p = '\0';
        return (i32)(p - buf);
    }

    inline i32 sprintf(char* buf, const char* format, ...) noexcept {
        va_list args;
        va_start(args, format);
        i32 result = core::vsnprintf(buf, Megabytes(1), format, args);
        va_end(args);
        return result;
    }

    inline i32 snprintf(char* buf, usize buf_size, const char* format, ...) noexcept {
        va_list args;
        va_start(args, format);
        i32 result = core::vsnprintf(buf, buf_size, format, args);
        va_end(args);
        return result;
    }
}

#include "core/memory/arena.h"
#include "core/string8.h"
#include "core/path.h"

#endif // CORE_H

