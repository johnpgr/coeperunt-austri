#ifndef WINDOW_H
#define WINDOW_H

#include "../core.h"

// Opaque type for SystemWindow
typedef struct PlatformWindow PlatformWindow;

typedef struct WindowApi {
    PlatformWindow* (*create)(const char* title, i32 width, i32 height) NOEXCEPT;
    void (*destroy)(PlatformWindow* window) NOEXCEPT;
    b8 (*poll)(PlatformWindow* window, i32* out_width, i32* out_height, b8* out_quit) NOEXCEPT;
} WindowApi;

EXTERN_C void window_init(WindowApi* window) NOEXCEPT;

#endif // WINDOW_H
