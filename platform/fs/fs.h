#ifndef PLATFORM_FS_H
#define PLATFORM_FS_H

#include "../../core.h"

typedef struct FsApi {
    String8 (*read_entire_file)(MemoryArena* arena, Path filepath) noexcept;
    b8 (*write_entire_file)(const char* filepath, const void* data, usize size) noexcept;
} FsApi;

#endif // PLATFORM_FS_H
