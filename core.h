#ifndef CORE_H
#define CORE_H

// ============================================================================
// Compile-Time OS Detection & Linking Directives
// ============================================================================
#if defined(_WIN32)
    #define OS_WINDOWS
    #pragma comment(lib, "kernel32.lib")
#elif defined(__linux__)
    #define OS_LINUX
#else
    #error "Unsupported Operating System"
#endif

#define EXTERN_C extern "C"
#define NOEXCEPT noexcept
#define NORETURN [[noreturn]]

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

// ============================================================================
// Custom String Utilities (No standard library dependencies)
// ============================================================================

/**
 * TODO: Create our own String type, that does not heap allocate and
 * that is going to be O(1) for length retrieval, and O(1) for slicing (substrings).
 */
inline constexpr usize string_len(const char* str) {
    usize len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
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

// ============================================================================
// Freestanding CRT Overrides (No-stdlib compiler safeties)
// ============================================================================
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

#endif // CORE_H

