#include "media.h"
#include "../platform.h"

LoadedImage linux_load_bmp(const char* filepath) noexcept {
    LoadedImage result = {};

    MemoryArena temp_arena = arena_alloc(Megabytes(32));

    Path path = path_parse(&temp_arena, str8_cstring(filepath));
    String8 file = api.fs.read_entire_file(&temp_arena, path);
    if (file.size < sizeof(BMPHeader) + sizeof(BMPInfoHeader)) {
        arena_release(&temp_arena);
        return result;
    }

    u8* bytes         = file.str;
    BMPHeader* header = (BMPHeader*)bytes;
    if (header->signature != 0x4D42) { // 'BM'
        core::printf("[Error] File is not a valid BMP image: %s\n", filepath);
        arena_release(&temp_arena);
        return result;
    }

    BMPInfoHeader* info = (BMPInfoHeader*)(bytes + sizeof(BMPHeader));
    if (info->bit_count != 32) {
        core::printf(
            "[Error] BMP is not 32-bit (RGBA). Only 32-bit BMPs are supported: "
            "%s (got %d-bit)\n",
            filepath,
            info->bit_count
        );
        arena_release(&temp_arena);
        return result;
    }

    i32 width      = info->width;
    i32 height     = info->height;
    i32 abs_height = (height < 0) ? -height : height;
    u8* src_pixels = bytes + header->data_offset;

    usize alloc_size = (usize)width * (usize)abs_height * 4;

    void* pixel_buffer = sys::mmap(
        nullptr,
        alloc_size,
        LINUX_PROT_READ | LINUX_PROT_WRITE,
        LINUX_MAP_PRIVATE | LINUX_MAP_ANONYMOUS,
        -1,
        0
    );

    if (pixel_buffer == LINUX_MAP_FAILED) {
        arena_release(&temp_arena);
        return result;
    }

    u8* dest = (u8*)pixel_buffer;
    if (height > 0) {
        // Bottom-up: copy and flip rows using optimized core::memcpy
        for (i32 y = 0; y < abs_height; ++y) {
            u8* src_row  = src_pixels + (abs_height - 1 - y) * (width * 4);
            u8* dest_row = dest + y * (width * 4);
            core::memcpy(dest_row, src_row, width * 4);
        }
    } else {
        // Top-down: copy directly using optimized core::memcpy
        core::memcpy(dest, src_pixels, width * abs_height * 4);
    }

    // Free the raw file memory arena immediately
    arena_release(&temp_arena);

    result.pixels      = dest;
    result.width       = width;
    result.height      = abs_height;
    result.file_memory = pixel_buffer;

    core::printf(
        "[System] Generic BMP Loader loaded: %s (%dx%d, 32-bit)\n",
        filepath,
        width,
        abs_height
    );
    return result;
}

void linux_free_bmp(LoadedImage image) noexcept {
    if (image.file_memory) {
        usize size = (usize)image.width * (usize)image.height * 4;
        sys::munmap(image.file_memory, size);
    }
}
