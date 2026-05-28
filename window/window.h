#ifndef WINDOW_H
#define WINDOW_H

#include "../core.h"

// Opaque type for SystemWindow
typedef struct PlatformWindow PlatformWindow;

typedef struct WindowApi {
    PlatformWindow* (*create)(const char* title, i32 width, i32 height) noexcept;
    void (*destroy)(PlatformWindow* window) noexcept;
    b8 (*poll)(PlatformWindow* window) noexcept;
} WindowApi;

EXTERN_C void window_init(WindowApi* window) noexcept;

#endif // WINDOW_H
