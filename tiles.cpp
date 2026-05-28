#include "tiles.h"

static Tile g_tile_buffer[MAP_WIDTH * MAP_HEIGHT];
// TODO: Fill this table with mask -> SpriteID mappings for the tileset.
static SpriteID g_autotile_lookup[256];

static b8 tilemap_is_solid(Tilemap* map, i32 x, i32 y) {
    Tile* tile = tilemap_tile(map, x, y);
    return (tile && tile->type == TILE_SOLID);
}

static u8 tilemask_compute(Tilemap* map, i32 x, i32 y) {
    if (!tilemap_is_solid(map, x, y)) return 0;

    // Bit order: N, E, S, W, NE, SE, SW, NW (diagonals gated by cardinals).
    b8 n  = tilemap_is_solid(map, x,     y - 1);
    b8 e  = tilemap_is_solid(map, x + 1, y);
    b8 s  = tilemap_is_solid(map, x,     y + 1);
    b8 w  = tilemap_is_solid(map, x - 1, y);
    b8 ne = tilemap_is_solid(map, x + 1, y - 1);
    b8 se = tilemap_is_solid(map, x + 1, y + 1);
    b8 sw = tilemap_is_solid(map, x - 1, y + 1);
    b8 nw = tilemap_is_solid(map, x - 1, y - 1);

    u8 mask = 0;
    if (n) mask |= (1 << 0);
    if (e) mask |= (1 << 1);
    if (s) mask |= (1 << 2);
    if (w) mask |= (1 << 3);
    if (n && e && ne) mask |= (1 << 4);
    if (s && e && se) mask |= (1 << 5);
    if (s && w && sw) mask |= (1 << 6);
    if (n && w && nw) mask |= (1 << 7);

    return mask;
}

void autotile_init(SpriteID fill) noexcept {
    for (i32 i = 0; i < 256; ++i) {
        g_autotile_lookup[i] = fill;
    }
}

void autotile_set(u8 mask, SpriteID sprite) noexcept {
    g_autotile_lookup[mask] = sprite;
}

Tile* tilemap_tile(Tilemap* map, i32 x, i32 y) noexcept {
    if (!map) return nullptr;
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) return nullptr;
    return map->tiles + (y * map->width + x);
}

static void tilemap_refresh(Tilemap* map, i32 x, i32 y) {
    Tile* tile = tilemap_tile(map, x, y);
    if (!tile) return;

    if (tile->type != TILE_SOLID) {
        tile->mask = 0;
        tile->sprite = SPRITE_EMPTY;
        return;
    }

    tile->mask = tilemask_compute(map, x, y);
    tile->sprite = g_autotile_lookup[tile->mask];
}

void tilemap_set(Tilemap* map, i32 x, i32 y, u8 type) noexcept {
    Tile* tile = tilemap_tile(map, x, y);
    if (!tile) return;
    if (tile->type == type) return;

    tile->type = type;
    for (i32 dy = -1; dy <= 1; ++dy) {
        for (i32 dx = -1; dx <= 1; ++dx) {
            tilemap_refresh(map, x + dx, y + dy);
        }
    }
}

void tilemap_init(Tilemap* map) noexcept {
    if (!map) return;
    map->width = MAP_WIDTH;
    map->height = MAP_HEIGHT;
    map->tiles = g_tile_buffer;

    i32 total = map->width * map->height;
    for (i32 i = 0; i < total; ++i) {
        map->tiles[i].type = TILE_EMPTY;
        map->tiles[i].mask = 0;
        map->tiles[i].sprite = SPRITE_EMPTY;
    }
}
