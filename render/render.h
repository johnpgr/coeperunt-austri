#ifndef RENDER_H
#define RENDER_H

#include "../core.h"
#include "../window/window.h"

// Opaque type for GraphicsContext
typedef struct GraphicsContext GraphicsContext;

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

typedef struct RenderApi {
    GraphicsContext* (*init)(PlatformWindow* window) noexcept;
    b8 (*upload_texture)(GraphicsContext* context, const u8* pixels, i32 width, i32 height) noexcept;
    void (*submit_frame)(PlatformWindow* window, GraphicsContext* context, RenderCommandQueue queue) noexcept;
    void (*destroy)(GraphicsContext* context) noexcept;
} RenderApi;

EXTERN_C void render_init(RenderApi* render) noexcept;

#endif // RENDER_H


