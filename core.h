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
    #define START EXTERN_C void no_crt_entry()
#elif defined(OS_LINUX)
    #define START EXTERN_C void nomain_entry(int argc, char** argv, char** envp)
#endif

#endif // CORE_H
