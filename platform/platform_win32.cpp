#include "platform.h"

#if defined(OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Custom symbols for no-stdlib environment
extern "C" {
    int _fltused = 0;
}

// ============================================================================
// Global functions implementation
// ============================================================================
namespace core {
    NORETURN void exit(i32 status) noexcept {
        ExitProcess((u32)status);
        while (1) {} // Guarantee termination
    }

    void print(const char* message) noexcept {
        if (message) {
            OutputDebugStringA(message);
        }
    }

    void printv(const char* format, va_list args) noexcept {
        char buf[2048];
        va_list args_copy;
        va_copy(args_copy, args);
        core::vsnprintf(buf, sizeof(buf), format, args_copy);
        va_end(args_copy);
        core::print(buf);
    }

    void printf(const char* format, ...) noexcept {
        va_list args;
        va_start(args, format);
        core::printv(format, args);
        va_end(args);
    }
}

// ============================================================================
// File System Implementation
// ============================================================================
static FileContent __win32_read_entire_file(const char* filepath) noexcept {
    FileContent result = {};
    HANDLE file = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        core::printf("[Error] Failed to open file: %s\n", filepath);
        return result;
    }
    
    LARGE_INTEGER size;
    if (GetFileSizeEx(file, &size)) {
        void* data = VirtualAlloc(nullptr, (SIZE_T)size.QuadPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (data) {
            DWORD bytes_read = 0;
            if (ReadFile(file, data, (DWORD)size.QuadPart, &bytes_read, nullptr) && bytes_read == size.QuadPart) {
                result.data = data;
                result.size = (usize)size.QuadPart;
            } else {
                core::printf("[Error] Failed to read file content: %s\n", filepath);
                VirtualFree(data, 0, MEM_RELEASE);
            }
        }
    }
    CloseHandle(file);
    return result;
}

static b8 __win32_write_entire_file(const char* filepath, const void* data, usize size) noexcept {
    HANDLE file = CreateFileA(filepath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        core::printf("[Error] Failed to open file for writing: %s\n", filepath);
        return false;
    }
    
    DWORD bytes_written = 0;
    b8 success = WriteFile(file, data, (DWORD)size, &bytes_written, nullptr) && (bytes_written == size);
    CloseHandle(file);
    return success;
}

static void __win32_free_file_content(FileContent content) noexcept {
    if (content.data) {
        VirtualFree(content.data, 0, MEM_RELEASE);
    }
}

// ============================================================================
// Media Parsing (BMP)
// ============================================================================
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

static LoadedImage __win32_load_bmp(const char* filepath) noexcept {
    LoadedImage result = {};
    
    // Use the file system platform API internally
    FileContent file = __win32_read_entire_file(filepath);
    if (!file.data || file.size < sizeof(BMPHeader) + sizeof(BMPInfoHeader)) {
        return result;
    }
    
    u8* bytes = (u8*)file.data;
    BMPHeader* header = (BMPHeader*)bytes;
    if (header->signature != 0x4D42) { // 'BM'
        core::printf("[Error] File is not a valid BMP image: %s\n", filepath);
        __win32_free_file_content(file);
        return result;
    }
    
    BMPInfoHeader* info = (BMPInfoHeader*)(bytes + sizeof(BMPHeader));
    if (info->bit_count != 32) {
        core::printf("[Error] BMP is not 32-bit (RGBA). Only 32-bit BMPs are supported: %s (got %d-bit)\n", filepath, info->bit_count);
        __win32_free_file_content(file);
        return result;
    }
    
    i32 width = info->width;
    i32 height = info->height;
    i32 abs_height = (height < 0) ? -height : height;
    u8* src_pixels = bytes + header->data_offset;
    
    // Allocate compact pixel buffer using VirtualAlloc
    void* pixel_buffer = VirtualAlloc(nullptr, width * abs_height * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pixel_buffer) {
        __win32_free_file_content(file);
        return result;
    }
    
    u8* dest = (u8*)pixel_buffer;
    if (height > 0) {
        // Bottom-up: copy and flip rows using optimized core::memcpy
        for (i32 y = 0; y < abs_height; ++y) {
            u8* src_row = src_pixels + (abs_height - 1 - y) * (width * 4);
            u8* dest_row = dest + y * (width * 4);
            core::memcpy(dest_row, src_row, width * 4);
        }
    } else {
        // Top-down: copy directly using optimized core::memcpy
        core::memcpy(dest, src_pixels, width * abs_height * 4);
    }
    
    // Free the raw file memory immediately
    __win32_free_file_content(file);
    
    result.pixels = dest;
    result.width = width;
    result.height = abs_height;
    result.file_memory = pixel_buffer;
    
    core::printf("[System] Generic BMP Loader loaded: %s (%dx%d, 32-bit)\n", filepath, width, abs_height);
    return result;
}

static void __win32_free_bmp(LoadedImage image) noexcept {
    if (image.file_memory) {
        VirtualFree(image.file_memory, 0, MEM_RELEASE);
    }
}

// Physically separate implementations included in Unity Build order
#include "../window/window_win32.cpp"
#include "../render/render_win32.cpp"

void platform_init(PlatformApi* api) noexcept {
    if (api) {
        api->window.create  = __win32_create_window;
        api->window.destroy = __win32_destroy_window;
        api->window.poll    = __win32_poll_events;

        api->render.init           = __win32_init_graphics;
        api->render.upload_texture = __win32_upload_texture;
        api->render.submit_frame   = __win32_submit_frame;
        api->render.destroy        = __win32_destroy_graphics;

        api->fs.read_entire_file  = __win32_read_entire_file;
        api->fs.write_entire_file = __win32_write_entire_file;
        api->fs.free_file_content = __win32_free_file_content;

        api->media.load_bmp = __win32_load_bmp;
        api->media.free_bmp = __win32_free_bmp;
    }
}

#endif // OS_WINDOWS
