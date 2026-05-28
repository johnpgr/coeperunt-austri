#ifndef SPRITES_H
#define SPRITES_H

#include "core.h"
#include "platform/platform.h"

enum SpriteID {
    // Systematic grid-based naming (8 rows, 5 columns)
    // Row 0
    SPRITE_TILE_R0_C0,
    SPRITE_TILE_R0_C1,
    SPRITE_TILE_R0_C2,
    SPRITE_TILE_R0_C3,
    SPRITE_TILE_R0_C4,

    // Row 1
    SPRITE_TILE_R1_C0,
    SPRITE_TILE_R1_C1,
    SPRITE_TILE_R1_C2,
    SPRITE_TILE_R1_C3,
    SPRITE_TILE_R1_C4,

    // Row 2
    SPRITE_TILE_R2_C0,
    SPRITE_TILE_R2_C1,
    SPRITE_TILE_R2_C2,
    SPRITE_TILE_R2_C3,
    SPRITE_TILE_R2_C4,

    // Row 3
    SPRITE_TILE_R3_C0,
    SPRITE_TILE_R3_C1,
    SPRITE_TILE_R3_C2,
    SPRITE_TILE_R3_C3,
    SPRITE_TILE_R3_C4,

    // Row 4
    SPRITE_TILE_R4_C0,
    SPRITE_TILE_R4_C1,
    SPRITE_TILE_R4_C2,
    SPRITE_TILE_R4_C3,
    SPRITE_TILE_R4_C4,

    // Row 5
    SPRITE_TILE_R5_C0,
    SPRITE_TILE_R5_C1,
    SPRITE_TILE_R5_C2,
    SPRITE_TILE_R5_C3,
    SPRITE_TILE_R5_C4,

    // Row 6
    SPRITE_TILE_R6_C0,
    SPRITE_TILE_R6_C1,
    SPRITE_TILE_R6_C2,
    SPRITE_TILE_R6_C3,
    SPRITE_TILE_R6_C4,

    // Row 7
    SPRITE_TILE_R7_C0,
    SPRITE_TILE_R7_C1,
    SPRITE_TILE_R7_C2,
    SPRITE_TILE_R7_C3,
    SPRITE_TILE_R7_C4,

    SPRITE_COUNT,

    // -------------------------------------------------------------------------
    // Semantic aliases for convenient auto-tiling and clarity
    // -------------------------------------------------------------------------
    
    // Empty / grey filler tile
    SPRITE_EMPTY = SPRITE_TILE_R0_C0,

    // Pit/Hole borders and corners (Row 0-2, Columns 1-3)
    SPRITE_PIT_TOP_LEFT     = SPRITE_TILE_R0_C1,
    SPRITE_PIT_TOP          = SPRITE_TILE_R0_C2,
    SPRITE_PIT_TOP_RIGHT    = SPRITE_TILE_R0_C3,
    SPRITE_PIT_LEFT         = SPRITE_TILE_R1_C1,
    SPRITE_PIT_RIGHT        = SPRITE_TILE_R1_C3,
    SPRITE_PIT_BOTTOM_LEFT  = SPRITE_TILE_R2_C1,
    SPRITE_PIT_BOTTOM       = SPRITE_TILE_R2_C2,
    SPRITE_PIT_BOTTOM_RIGHT = SPRITE_TILE_R2_C3,

    // Transitions and inner corners (Row 3 & 5, Columns 1-3)
    SPRITE_INNER_TOP_LEFT     = SPRITE_TILE_R3_C1,
    SPRITE_INNER_TOP          = SPRITE_TILE_R3_C2,
    SPRITE_INNER_TOP_RIGHT    = SPRITE_TILE_R3_C3,
    SPRITE_INNER_BOTTOM_LEFT  = SPRITE_TILE_R5_C1,
    SPRITE_INNER_BOTTOM       = SPRITE_TILE_R5_C2,
    SPRITE_INNER_BOTTOM_RIGHT = SPRITE_TILE_R5_C3,

    // Clean Grass variations (Column 0, Rows 1-5)
    SPRITE_GRASS_A = SPRITE_TILE_R1_C0,
    SPRITE_GRASS_B = SPRITE_TILE_R2_C0,
    SPRITE_GRASS_C = SPRITE_TILE_R3_C0,
    SPRITE_GRASS_D = SPRITE_TILE_R4_C0,
    SPRITE_GRASS_E = SPRITE_TILE_R5_C0,

    // Solid Dirt / Underground variations
    SPRITE_DIRT       = SPRITE_TILE_R2_C4, // Default solid dirt
    SPRITE_DIRT_VAR1  = SPRITE_TILE_R0_C4,
    SPRITE_DIRT_VAR2  = SPRITE_TILE_R1_C2,
    SPRITE_DIRT_VAR3  = SPRITE_TILE_R1_C4,
    SPRITE_DIRT_VAR4  = SPRITE_TILE_R3_C4,
    SPRITE_DIRT_VAR5  = SPRITE_TILE_R4_C1,
    SPRITE_DIRT_VAR6  = SPRITE_TILE_R4_C2,
    SPRITE_DIRT_VAR7  = SPRITE_TILE_R4_C3,
    SPRITE_DIRT_VAR8  = SPRITE_TILE_R4_C4,
    SPRITE_DIRT_VAR9  = SPRITE_TILE_R5_C4,

    // Grass with flowers / details (Row 6 & 7, Columns 0-4)
    SPRITE_FLOWERS_WHITE_A = SPRITE_TILE_R6_C0,
    SPRITE_FLOWERS_WHITE_B = SPRITE_TILE_R6_C1,
    SPRITE_FLOWERS_WHITE_C = SPRITE_TILE_R6_C3,
    SPRITE_FLOWERS_YELLOW  = SPRITE_TILE_R6_C2,
    SPRITE_GRASS_STONES    = SPRITE_TILE_R6_C4,

    SPRITE_FLOWERS_VAR1    = SPRITE_TILE_R7_C0,
    SPRITE_FLOWERS_VAR2    = SPRITE_TILE_R7_C1,
    SPRITE_FLOWERS_VAR3    = SPRITE_TILE_R7_C2,
    SPRITE_FLOWERS_VAR4    = SPRITE_TILE_R7_C3,
    SPRITE_GRASS_STONE_MED = SPRITE_TILE_R7_C4,
};

struct SpriteDef {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
    i32 frame_count;
};

static const SpriteDef g_sprite_defs[SPRITE_COUNT] = {
    // Row 0
    [SPRITE_TILE_R0_C0] = { .x = 0,   .y = 0,   .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R0_C1] = { .x = 16,  .y = 0,   .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R0_C2] = { .x = 32,  .y = 0,   .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R0_C3] = { .x = 48,  .y = 0,   .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R0_C4] = { .x = 64,  .y = 0,   .w = 16, .h = 16, .frame_count = 1 },

    // Row 1
    [SPRITE_TILE_R1_C0] = { .x = 0,   .y = 16,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R1_C1] = { .x = 16,  .y = 16,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R1_C2] = { .x = 32,  .y = 16,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R1_C3] = { .x = 48,  .y = 16,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R1_C4] = { .x = 64,  .y = 16,  .w = 16, .h = 16, .frame_count = 1 },

    // Row 2
    [SPRITE_TILE_R2_C0] = { .x = 0,   .y = 32,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R2_C1] = { .x = 16,  .y = 32,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R2_C2] = { .x = 32,  .y = 32,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R2_C3] = { .x = 48,  .y = 32,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R2_C4] = { .x = 64,  .y = 32,  .w = 16, .h = 16, .frame_count = 1 },

    // Row 3
    [SPRITE_TILE_R3_C0] = { .x = 0,   .y = 48,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R3_C1] = { .x = 16,  .y = 48,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R3_C2] = { .x = 32,  .y = 48,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R3_C3] = { .x = 48,  .y = 48,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R3_C4] = { .x = 64,  .y = 48,  .w = 16, .h = 16, .frame_count = 1 },

    // Row 4
    [SPRITE_TILE_R4_C0] = { .x = 0,   .y = 64,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R4_C1] = { .x = 16,  .y = 64,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R4_C2] = { .x = 32,  .y = 64,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R4_C3] = { .x = 48,  .y = 64,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R4_C4] = { .x = 64,  .y = 64,  .w = 16, .h = 16, .frame_count = 1 },

    // Row 5
    [SPRITE_TILE_R5_C0] = { .x = 0,   .y = 80,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R5_C1] = { .x = 16,  .y = 80,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R5_C2] = { .x = 32,  .y = 80,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R5_C3] = { .x = 48,  .y = 80,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R5_C4] = { .x = 64,  .y = 80,  .w = 16, .h = 16, .frame_count = 1 },

    // Row 6
    [SPRITE_TILE_R6_C0] = { .x = 0,   .y = 96,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R6_C1] = { .x = 16,  .y = 96,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R6_C2] = { .x = 32,  .y = 96,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R6_C3] = { .x = 48,  .y = 96,  .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R6_C4] = { .x = 64,  .y = 96,  .w = 16, .h = 16, .frame_count = 1 },

    // Row 7
    [SPRITE_TILE_R7_C0] = { .x = 0,   .y = 112, .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R7_C1] = { .x = 16,  .y = 112, .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R7_C2] = { .x = 32,  .y = 112, .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R7_C3] = { .x = 48,  .y = 112, .w = 16, .h = 16, .frame_count = 1 },
    [SPRITE_TILE_R7_C4] = { .x = 64,  .y = 112, .w = 16, .h = 16, .frame_count = 1 },
};

inline TextureRegion sprite_region(SpriteID id, i32 atlas_width, i32 atlas_height) {
    SpriteDef def = g_sprite_defs[id];
    TextureRegion region = {};
    region.u0 = (f32)def.x / (f32)atlas_width;
    region.v0 = (f32)def.y / (f32)atlas_height;
    region.u1 = (f32)(def.x + def.w) / (f32)atlas_width;
    region.v1 = (f32)(def.y + def.h) / (f32)atlas_height;
    region.width = def.w;
    region.height = def.h;
    return region;
}

#endif // SPRITES_H
