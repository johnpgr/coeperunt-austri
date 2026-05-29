#ifndef CORE_PATH_H
#define CORE_PATH_H

#include "../core.h"

enum PathStyle {
    PathStyle_Unix,
    PathStyle_Windows,
    PathStyle_Native
};

struct Path {
    String8 header_string;
    i32 number_of_leading_slashes;
    b8 trailing_slash;
    String8* words;
    u32 word_count;
};

// Parses a path zero-copy (pointing words into the input string).
EXTERN_C Path path_parse(MemoryArena* arena, String8 input) noexcept;

// Performs a deep copy of the Path structure and its strings into the arena.
EXTERN_C Path path_copy(MemoryArena* arena, Path path) noexcept;

// Resolves "." and ".." components (path normalization).
EXTERN_C Path path_reduce(MemoryArena* arena, Path path) noexcept;

// Serializes a path into a flat String8 based on the requested style.
EXTERN_C String8 path_to_string(MemoryArena* arena, Path path, PathStyle style) noexcept;

// Combines two paths, automatically handling absolute/relative path semantics.
EXTERN_C Path path_join(MemoryArena* arena, Path base, Path tail) noexcept;

#endif // CORE_PATH_H
