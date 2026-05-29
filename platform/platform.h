#ifndef PLATFORM_H
#define PLATFORM_H

#include "../core.h"
#include "fs/fs.h"
#include "media/media.h"
#include "window/window.h"
#include "../render/render.h"

// Opaque types
typedef struct PlatformWindow PlatformWindow;
typedef struct GraphicsContext GraphicsContext;

// ============================================================================
// Central Platform API Definition
// ============================================================================
typedef struct PlatformApi {
    b8 quit;

    struct {
        PlatformWindow* (*create)(const char* title, i32 width, i32 height) noexcept;
        void (*destroy)(PlatformWindow* window) noexcept;
        b8 (*poll)(PlatformWindow* window) noexcept;
        i32 width;
        i32 height;
    } window;

    struct {
        struct {
            i32 x;
            i32 y;
            b8 left_down;
            b8 right_down;
        } mouse;

        struct {
            b8 keys[256];
        } keyboard;
    } input;

    RenderApi render;
    FsApi fs;
    MediaApi media;
} PlatformApi;

// Global instance definition
extern PlatformApi api;

// ============================================================================
// Global functions
// ============================================================================
EXTERN_C void platform_init(PlatformApi* api) noexcept;

#endif // PLATFORM_H
