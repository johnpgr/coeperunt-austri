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
} FileContent;

typedef struct TextureRegion {
    f32 u0, v0; // Top-left UV coordinates in [0, 1]
    f32 u1, v1; // Bottom-right UV coordinates in [0, 1]
    i32 width;  // Pixel width of the region
    i32 height; // Pixel height of the region
} TextureRegion;

// ============================================================================
// Render Command Queue & Push Buffer Structures
// ============================================================================
typedef enum RenderCommandKind {
    RENDER_COMMAND_CLEAR,
    RENDER_COMMAND_QUAD,
} RenderCommandKind;

typedef struct RenderCommandHeader {
    RenderCommandKind kind;
    u32 size; // Total size of the command block (including header and payload)
} RenderCommandHeader;

typedef struct RenderCommandClear {
    f32 r, g, b, a;
} RenderCommandClear;

typedef struct RenderCommandQuad {
    f32 pos_x, pos_y;
    f32 scale_x, scale_y;
    f32 origin_x, origin_y;
    f32 uv_x, uv_y;
    f32 uv_w, uv_h;
    f32 rotation;
    f32 color_r, color_g, color_b, color_a;
} RenderCommandQuad;

typedef struct RenderCommandQueue {
    u8* buffer;
    usize capacity;
    usize size;
} RenderCommandQueue;

// ============================================================================
// Central Platform API Definition
// ============================================================================
typedef struct PlatformApi {
    struct {
        PlatformWindow* (*create)(const char* title, i32 width, i32 height) noexcept;
        void (*destroy)(PlatformWindow* window) noexcept;
        b8 (*poll)(PlatformWindow* window, i32* out_width, i32* out_height, b8* out_quit) noexcept;
    } window;

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
