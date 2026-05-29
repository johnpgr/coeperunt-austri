#ifndef PLATFORM_MEDIA_H
#define PLATFORM_MEDIA_H

#include "../../core.h"

typedef struct LoadedImage {
    u8* pixels;
    i32 width;
    i32 height;
    void* file_memory;
} LoadedImage;

typedef struct MediaApi {
    LoadedImage (*load_bmp)(const char* filepath) noexcept;
    void (*free_bmp)(LoadedImage image) noexcept;
} MediaApi;

#endif // PLATFORM_MEDIA_H
