/**
 * TODO: auto-tiling with the tileset we have, using bitmasking to determine
 * which tile variation to use based on neighboring tiles
 */

#include "core.h"
#include "core/path.cpp"

#if defined(OS_WINDOWS)
#include "platform/platform_win32.cpp"
#elif defined(OS_LINUX)
#include "platform/platform_linux.cpp"
#endif

#define FOREST_TILESET_PATH "assets/tilesets/forest.bmp"

PlatformApi api;

#include "sprites.h"
#include "tiles.h"
#include "tiles.cpp"

MAIN {
    platform_init(&api);
    core::printf("[System] Platform API initialized.\n");

    api.window.width  = 800;
    api.window.height = 600;
    core::printf(
        "[System] Creating window (%dx%d)...\n",
        api.window.width,
        api.window.height
    );

    PlatformWindow* windowptr =
        api.window.create("Unnamed Game", api.window.width, api.window.height);
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

    i32 atlas_width  = atlas_image.width;
    i32 atlas_height = atlas_image.height;

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

    TextureRegion sprite_regions[SPRITE_COUNT] = {};
    for (i32 i = 0; i < SPRITE_COUNT; ++i) {
        sprite_regions[i] =
            sprite_region((SpriteID)i, atlas_width, atlas_height);
    }

    Tilemap tilemap = {};
    tilemap_init(&tilemap);
    autotile_init(SPRITE_DIRT);
    autotile_setup_rules();

    static u8 render_command_buffer[Megabytes(1)];
    RenderCommandQueue queue = {};
    queue.buffer             = render_command_buffer;
    queue.capacity           = sizeof(render_command_buffer);

    api.quit = false;

    core::printf(
        "[System] Subsystem initialized successfully. Running frame loop "
        "(Window: %p, Width: %d, Height: %d)...\n",
        windowptr,
        api.window.width,
        api.window.height
    );

    while (!api.quit) {
        api.window.poll(windowptr);

        if (api.input.mouse.left_down) {
            i32 cell_x = api.input.mouse.x / TILE_DRAW_SIZE;
            i32 cell_y = api.input.mouse.y / TILE_DRAW_SIZE;
            tilemap_set(&tilemap, cell_x, cell_y, TILE_SOLID);
        } else if (api.input.mouse.right_down) {
            i32 cell_x = api.input.mouse.x / TILE_DRAW_SIZE;
            i32 cell_y = api.input.mouse.y / TILE_DRAW_SIZE;
            tilemap_set(&tilemap, cell_x, cell_y, TILE_EMPTY);
        }

        queue.size = 0;
        push_clear(&queue, 0.45f, 0.68f, 0.90f, 1.0f);

        i32 draw_cols = api.window.width / TILE_DRAW_SIZE;
        i32 draw_rows = api.window.height / TILE_DRAW_SIZE;
        if (draw_cols > tilemap.width) draw_cols = tilemap.width;
        if (draw_rows > tilemap.height) draw_rows = tilemap.height;

        for (i32 y = 0; y < draw_rows; ++y) {
            for (i32 x = 0; x < draw_cols; ++x) {
                Tile* tile = tilemap_tile(&tilemap, x, y);
                if (!tile || tile->type == TILE_EMPTY) {
                    continue;
                }

                TextureRegion region = sprite_regions[tile->sprite];
                f32 pos_x            = (f32)(x * TILE_DRAW_SIZE);
                f32 pos_y            = (f32)(y * TILE_DRAW_SIZE);

                push_quad(&queue, region,
                    pos_x, pos_y,
                    (f32)TILE_DRAW_SCALE,
                    (f32)TILE_DRAW_SCALE,
                    0.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 1.0f, 1.0f
                );
            }
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
