#ifndef PLATFORM_H
#define PLATFORM_H

#include "../core.h"

// Opaque types
typedef struct PlatformWindow PlatformWindow;
typedef struct GraphicsContext GraphicsContext;

// Data structures
typedef struct LoadedImage {
    u8* pixels;
    i32 width;
    i32 height;
    void* file_memory;
} LoadedImage;

typedef struct FileContent {
    void* data;
    usize size;
    void* file_memory;
    usize file_memory_size;
} FileContent;

#include "../render/render.h"

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

    struct {
        GraphicsContext* (*init)(PlatformWindow* window) noexcept;
        b8 (*upload_texture)(GraphicsContext* context, const u8* pixels, i32 width, i32 height) noexcept;
        void (*submit_frame)(PlatformWindow* window, GraphicsContext* context, RenderCommandQueue queue) noexcept;
        void (*destroy)(GraphicsContext* context) noexcept;
    } render;

    struct {
        FileContent (*read_entire_file)(const char* filepath) noexcept;
        b8 (*write_entire_file)(const char* filepath, const void* data, usize size) noexcept;
        void (*free_file_content)(FileContent content) noexcept;
    } fs;

    struct {
        LoadedImage (*load_bmp)(const char* filepath) noexcept;
        void (*free_bmp)(LoadedImage image) noexcept;
    } media;
} PlatformApi;

// Global instance definition
extern PlatformApi api;

// ============================================================================
// Global functions
// ============================================================================
EXTERN_C void platform_init(PlatformApi* api) noexcept;

#endif // PLATFORM_H
