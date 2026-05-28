#ifndef RENDER_H
#define RENDER_H

#include "../core.h"
#include "../window/window.h"

// Opaque type for GraphicsContext
typedef struct GraphicsContext GraphicsContext;

typedef struct RenderApi {
    GraphicsContext* (*init)(PlatformWindow* window) NOEXCEPT;
    void (*clear)(GraphicsContext* context, f32 r, f32 g, f32 b, f32 a) NOEXCEPT;
    void (*swap)(PlatformWindow* window, GraphicsContext* context) NOEXCEPT;
    void (*destroy)(GraphicsContext* context) NOEXCEPT;
} RenderApi;

EXTERN_C void render_init(RenderApi* render) NOEXCEPT;

#endif // RENDER_H
