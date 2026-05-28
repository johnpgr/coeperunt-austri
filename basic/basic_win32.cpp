#include "basic.h"

#if defined(OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Custom symbols for no-stdlib environment
extern "C" {
    int _fltused = 0;
}

static void __win32_debug_log(const char* message) NOEXCEPT {
    if (message) {
        OutputDebugStringA(message);
    }
}

static void __win32_debug_logv(const char* format, va_list args) NOEXCEPT {
    char buf[2048];
    va_list args_copy;
    va_copy(args_copy, args);
    custom_vsnprintf(buf, sizeof(buf), format, args_copy);
    va_end(args_copy);
    __win32_debug_log(buf);
}

static void __win32_debug_logf(const char* format, ...) NOEXCEPT {
    va_list args;
    va_start(args, format);
    __win32_debug_logv(format, args);
    va_end(args);
}

NORETURN static void __win32_exit(i32 status) NOEXCEPT {
    ExitProcess((u32)status);
    while (1) {} // Guarantee termination
}

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

static LoadedImage __win32_load_bmp(const char* filepath) NOEXCEPT {
    LoadedImage result = {};
    
    // 1. Open the file via Win32 API
    HANDLE file = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        __win32_debug_logf("[Error] Failed to open BMP file: %s\n", filepath);
        return result;
    }
    
    LARGE_INTEGER file_size_struct;
    if (!GetFileSizeEx(file, &file_size_struct)) {
        CloseHandle(file);
        return result;
    }
    
    usize file_size = (usize)file_size_struct.QuadPart;
    
    // 2. Allocate buffer using VirtualAlloc
    void* file_memory = VirtualAlloc(nullptr, file_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!file_memory) {
        CloseHandle(file);
        return result;
    }
    
    // 3. Read the file
    DWORD bytes_read = 0;
    if (!ReadFile(file, file_memory, (DWORD)file_size, &bytes_read, nullptr) || bytes_read != file_size) {
        VirtualFree(file_memory, 0, MEM_RELEASE);
        CloseHandle(file);
        return result;
    }
    CloseHandle(file);
    
    // 4. Validate BMP signature
    u8* bytes = (u8*)file_memory;
    BMPHeader* header = (BMPHeader*)bytes;
    if (header->signature != 0x4D42) { // 'BM'
        __win32_debug_logf("[Error] File is not a valid BMP image: %s\n", filepath);
        VirtualFree(file_memory, 0, MEM_RELEASE);
        return result;
    }
    
    BMPInfoHeader* info = (BMPInfoHeader*)(bytes + sizeof(BMPHeader));
    if (info->bit_count != 32) {
        __win32_debug_logf("[Error] BMP is not 32-bit (RGBA). Only 32-bit BMPs are supported: %s (got %d-bit)\n", filepath, info->bit_count);
        VirtualFree(file_memory, 0, MEM_RELEASE);
        return result;
    }
    
    i32 width = info->width;
    i32 height = info->height;
    i32 abs_height = (height < 0) ? -height : height;
    u8* src_pixels = bytes + header->data_offset;
    
    // 5. Allocate compact pixel buffer using VirtualAlloc
    void* pixel_buffer = VirtualAlloc(nullptr, width * abs_height * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pixel_buffer) {
        VirtualFree(file_memory, 0, MEM_RELEASE);
        return result;
    }
    
    u8* dest = (u8*)pixel_buffer;
    if (height > 0) {
        // Bottom-up: copy and flip rows
        for (i32 y = 0; y < abs_height; ++y) {
            u8* src_row = src_pixels + (abs_height - 1 - y) * (width * 4);
            u8* dest_row = dest + y * (width * 4);
            for (i32 x = 0; x < width * 4; ++x) {
                dest_row[x] = src_row[x];
            }
        }
    } else {
        // Top-down: copy directly
        for (i32 i = 0; i < width * abs_height * 4; ++i) {
            dest[i] = src_pixels[i];
        }
    }
    
    // Free the raw file memory immediately
    VirtualFree(file_memory, 0, MEM_RELEASE);
    
    result.pixels = dest;
    result.width = width;
    result.height = abs_height;
    result.file_memory = pixel_buffer;
    
    __win32_debug_logf("[System] Generic BMP Loader loaded: %s (%dx%d, 32-bit)\n", filepath, width, abs_height);
    return result;
}

static void __win32_free_bmp(LoadedImage image) NOEXCEPT {
    if (image.file_memory) {
        VirtualFree(image.file_memory, 0, MEM_RELEASE);
    }
}

void basic_init(Basic* basic) NOEXCEPT {
    if (basic) {
        basic->print    = __win32_debug_log;
        basic->printf   = __win32_debug_logf;
        basic->printv   = __win32_debug_logv;
        basic->exit     = __win32_exit;
        basic->load_bmp = __win32_load_bmp;
        basic->free_bmp = __win32_free_bmp;
    }
}


#endif // OS_WINDOWS
