#include "glx_linux.h"

#include <dlfcn.h>

static void* g_opengl_lib = nullptr;

#define X(name, ret, params) PFN_##name name = nullptr;
GLX_PROC_LIST
#undef X

PFN_glXGetProcAddress glXGetProcAddress_ptr = nullptr;

#define GLX_LOAD_REQUIRED(symbol) \
    do { \
        symbol = (PFN_##symbol)dlsym(g_opengl_lib, #symbol); \
        if (!symbol) { \
            core::printf("[Error] Failed to load %s\n", #symbol); \
            return false; \
        } \
    } while (0)

static b8 glx_open_library() noexcept {
    if (g_opengl_lib) return true;

    g_opengl_lib = dlopen("libGL.so.1", RTLD_LAZY);
    if (!g_opengl_lib) {
        g_opengl_lib = dlopen("libGL.so", RTLD_LAZY);
    }
    if (!g_opengl_lib) {
        core::printf("[Error] Failed to load libGL.so\n");
        return false;
    }
    return true;
}

b8 glx_load_symbols() noexcept {
    b8 all_loaded = true;
    #define X(name, ret, params) if (!name) { all_loaded = false; }
    GLX_PROC_LIST
    #undef X
    if (all_loaded) return true;

    if (!glx_open_library()) {
        return false;
    }

    #define X(name, ret, params) GLX_LOAD_REQUIRED(name);
    GLX_PROC_LIST
    #undef X

    if (!glXGetProcAddress_ptr) {
        glXGetProcAddress_ptr = (PFN_glXGetProcAddress)dlsym(g_opengl_lib, "glXGetProcAddressARB");
        if (!glXGetProcAddress_ptr) {
            glXGetProcAddress_ptr = (PFN_glXGetProcAddress)dlsym(g_opengl_lib, "glXGetProcAddress");
        }
    }

    return true;
}

void* glx_dlsym(const char* name) noexcept {
    if (!glx_open_library()) {
        return nullptr;
    }
    return dlsym(g_opengl_lib, name);
}

void glx_unload_symbols() noexcept {
    if (g_opengl_lib) {
        dlclose(g_opengl_lib);
        g_opengl_lib = nullptr;
    }

    #define X(name, ret, params) name = nullptr;
    GLX_PROC_LIST
    #undef X
    glXGetProcAddress_ptr = nullptr;
}

#undef GLX_LOAD_REQUIRED
