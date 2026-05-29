#ifndef PLATFORM_MEDIA_H
#define PLATFORM_MEDIA_H

#include "../../core.h"

typedef struct LoadedImage {
    u8* pixels;
    i32 width;
    i32 height;
    void* file_memory;
} LoadedImage;

#pragma pack(push, 1)
struct BMPHeader {
    u16 signature;
    u32 file_size;
    u16 reserved1;
    u16 reserved2;
    u32 data_offset;
};

struct BMPInfoHeader {
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bit_count;
    u32 compression;
    u32 image_size;
    i32 x_pixels_per_m;
    i32 y_pixels_per_m;
    u32 colors_used;
    u32 colors_important;
};
#pragma pack(pop)

typedef struct MediaApi {
    LoadedImage (*load_bmp)(const char* filepath) noexcept;
    void (*free_bmp)(LoadedImage image) noexcept;
} MediaApi;

#endif // PLATFORM_MEDIA_H
