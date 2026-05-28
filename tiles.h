#ifndef TILES_H
#define TILES_H

#include "core.h"
#include "sprites.h"

static const i32 TILE_PIXEL_SIZE = 16;
static const i32 TILE_DRAW_SCALE = 2;
static const i32 TILE_DRAW_SIZE = TILE_PIXEL_SIZE * TILE_DRAW_SCALE;

static const i32 MAP_WIDTH = 64;
static const i32 MAP_HEIGHT = 64;

enum TileType : u8 {
    TILE_EMPTY = 0,
    TILE_SOLID = 1,
};

struct Tile {
    u8 type;
    u8 mask;
    SpriteID sprite;
};

struct Tilemap {
    i32 width;
    i32 height;
    Tile* tiles;
};

void autotile_init(SpriteID fill) noexcept;
void autotile_set(u8 mask, SpriteID sprite) noexcept;
void autotile_setup_rules() noexcept;

Tile* tilemap_tile(Tilemap* map, i32 x, i32 y) noexcept;
void tilemap_set(Tilemap* map, i32 x, i32 y, u8 type) noexcept;
void tilemap_init(Tilemap* map) noexcept;

#endif // TILES_H
