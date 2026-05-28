/**
* TODO: auto-tiling with the tileset we have, using bitmasking to determine
* which tile variation to use based on neighboring tiles
*/

#include "core.h"

#if defined(OS_WINDOWS)
#include "platform/platform_win32.cpp"
#elif defined(OS_LINUX)
#include "platform/platform_linux.cpp"
#endif

#define FOREST_TILESET_PATH "assets/tilesets/forest.bmp"

PlatformApi api;

#if defined(OS_LINUX)
EXTERN_C char** environ;
#endif

static void push_clear(RenderCommandQueue* queue, f32 r, f32 g, f32 b, f32 a) {
    usize cmd_size = sizeof(RenderCommandHeader) + sizeof(RenderCommandClear);
    if (queue->size + cmd_size > queue->capacity)
        return;

    RenderCommandHeader* header =
        (RenderCommandHeader*)(queue->buffer + queue->size);
    header->kind = RENDER_COMMAND_CLEAR;
    header->size = (u32)cmd_size;

    RenderCommandClear* payload =
        (RenderCommandClear*)(queue->buffer + queue->size +
                              sizeof(RenderCommandHeader));
    payload->r = r;
    payload->g = g;
    payload->b = b;
    payload->a = a;

    queue->size += cmd_size;
}

static void push_quad(
    RenderCommandQueue* queue,
    TextureRegion region,
    f32 x,
    f32 y,
    f32 scale_x,
    f32 scale_y,
    f32 rotation,
    f32 origin_x,
    f32 origin_y,
    f32 r,
    f32 g,
    f32 b,
    f32 a
) {
    usize cmd_size = sizeof(RenderCommandHeader) + sizeof(RenderCommandQuad);
    if (queue->size + cmd_size > queue->capacity)
        return;

    RenderCommandHeader* header =
        (RenderCommandHeader*)(queue->buffer + queue->size);
    header->kind = RENDER_COMMAND_QUAD;
    header->size = (u32)cmd_size;

    RenderCommandQuad* payload =
        (RenderCommandQuad*)(queue->buffer + queue->size +
                             sizeof(RenderCommandHeader));
    payload->pos_x    = x;
    payload->pos_y    = y;
    payload->scale_x  = scale_x * region.width;
    payload->scale_y  = scale_y * region.height;
    payload->origin_x = origin_x;
    payload->origin_y = origin_y;
    payload->uv_x     = region.u0;
    payload->uv_y     = region.v0;
    payload->uv_w     = region.u1 - region.u0;
    payload->uv_h     = region.v1 - region.v0;
    payload->rotation = rotation;
    payload->color_r  = r;
    payload->color_g  = g;
    payload->color_b  = b;
    payload->color_a  = a;

    queue->size += cmd_size;
}

MAIN {
#if defined(OS_LINUX)
    environ = envp;
#endif

    core::printf("[System] Booting...\n");

    platform_init(&api);
    core::printf("[System] Platform API initialized.\n");

    core::printf("[System] Creating window (800x600)...\n");

    PlatformWindow* windowptr = api.window.create("Unnamed Game", 800, 600);
    if (!windowptr) {
        core::printf("[Error] Failed to create window!\n");
        core::exit(1);
    }

    core::printf("[System] Window created: %p\n", windowptr);

    core::printf("[System] Initializing renderer...\n");
    GraphicsContext* context = api.render.init(windowptr);
    if (!context) {
        core::printf("[Error] Failed to initialize graphics context!\n");
        api.window.destroy(windowptr);
        core::exit(1);
    }

    core::printf("[System] Renderer initialized.\n");

    core::printf("[System] Loading texture atlas: %s\n", FOREST_TILESET_PATH);
    LoadedImage atlas_image = api.media.load_bmp(FOREST_TILESET_PATH);

    if (!atlas_image.pixels) {
        core::printf(
            "[Error] Failed to load forest tileset image from disk!\n"
        );
        api.render.destroy(context);
        api.window.destroy(windowptr);
        core::exit(1);
    }

    core::printf(
        "[System] Texture atlas loaded (%dx%d).\n",
        atlas_image.width,
        atlas_image.height
    );

    core::printf("[System] Uploading texture to GPU...\n");
    b8 upload_success = api.render.upload_texture(
        context,
        atlas_image.pixels,
        atlas_image.width,
        atlas_image.height
    );

    api.media.free_bmp(atlas_image);

    if (!upload_success) {
        core::printf(
            "[Error] Failed to upload forest tileset texture to GPU!\n"
        );
        api.render.destroy(context);
        api.window.destroy(windowptr);
        core::exit(1);
    }

    core::printf("[System] Texture upload complete.\n");

    // Define 32x32 texture region slices from the 384x256 forest tileset
    // Grass Variation A (Row 5, Column 1): x=0, y=128
    TextureRegion grass_a = {};
    grass_a.u0            = 0.0f / 384.0f;
    grass_a.v0            = 128.0f / 256.0f;
    grass_a.u1            = 32.0f / 384.0f;
    grass_a.v1            = 160.0f / 256.0f;
    grass_a.width         = 32;
    grass_a.height        = 32;

    // Grass Variation B (Row 5, Column 2): x=32, y=128
    TextureRegion grass_b = {};
    grass_b.u0            = 32.0f / 384.0f;
    grass_b.v0            = 128.0f / 256.0f;
    grass_b.u1            = 64.0f / 384.0f;
    grass_b.v1            = 160.0f / 256.0f;
    grass_b.width         = 32;
    grass_b.height        = 32;

    // Grass Variation C (Row 5, Column 3): x=64, y=128
    TextureRegion grass_c = {};
    grass_c.u0            = 64.0f / 384.0f;
    grass_c.v0            = 128.0f / 256.0f;
    grass_c.u1            = 96.0f / 384.0f;
    grass_c.v1            = 160.0f / 256.0f;
    grass_c.width         = 32;
    grass_c.height        = 32;

    // Dirt/Underground Tile (Row 2, Column 3): x=64, y=32
    TextureRegion dirt = {};
    dirt.u0            = 64.0f / 384.0f;
    dirt.v0            = 32.0f / 256.0f;
    dirt.u1            = 96.0f / 384.0f;
    dirt.v1            = 64.0f / 256.0f;
    dirt.width         = 32;
    dirt.height        = 32;

    static u8 render_command_buffer[1024 * 1024];
    RenderCommandQueue queue = {};
    queue.buffer             = render_command_buffer;
    queue.capacity           = sizeof(render_command_buffer);

    i32 width  = 800;
    i32 height = 600;
    b8 quit    = false;

    core::printf(
        "[System] Subsystem initialized successfully. Running frame loop "
        "(Window: %p, Width: %d, Height: %d)...\n",
        windowptr,
        width,
        height
    );

    while (!quit) {
        api.window.poll(windowptr, &width, &height, &quit);

        queue.size = 0;
        push_clear(&queue, 0.45f, 0.68f, 0.90f, 1.0f);

        for (int x = 0; x < 5; ++x) {
            f32 tile_x = (f32)(240 + x * 64);

            TextureRegion grass_tile =
                (x % 3 == 0) ? grass_a : ((x % 3 == 1) ? grass_b : grass_c);

            push_quad(
                &queue,
                grass_tile,
                tile_x,
                300.0f,
                2.0f,
                2.0f,
                0.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                1.0f,
                1.0f
            );

            push_quad(
                &queue,
                dirt,
                tile_x,
                364.0f,
                2.0f,
                2.0f,
                0.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                1.0f,
                1.0f
            );
        }

        api.render.submit_frame(windowptr, context, queue);
    }

    core::printf("[System] Window close requested. Cleaning up resources...\n");

    api.render.destroy(context);
    api.window.destroy(windowptr);

    core::printf(
        "[System] Subsystem shutdown complete. Exiting cleanly with code %d.\n",
        0
    );

    core::exit(0);
}
