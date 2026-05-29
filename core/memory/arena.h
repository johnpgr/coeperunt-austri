#ifndef CORE_MEMORY_ARENA_H
#define CORE_MEMORY_ARENA_H

#include "../../core.h"

struct MemoryArena {
    u8* base;
    usize size;
    usize used;
};

// Creates an arena by reserving/committing virtual memory from the OS.
EXTERN_C MemoryArena arena_alloc(usize size) noexcept;

// Releases the virtual memory backing the arena to the OS.
EXTERN_C void arena_release(MemoryArena* arena) noexcept;

// Core push operation
inline void* arena_push(MemoryArena* arena, usize size, usize align = 8, b32 zero = true) noexcept {
    if (!arena || !arena->base || size == 0) {
        return nullptr;
    }

    // Align the current used position
    usize alignment_mask = align - 1;
    usize aligned_used = (arena->used + alignment_mask) & ~alignment_mask;

    if (aligned_used + size > arena->size) {
        return nullptr;
    }

    void* result = arena->base + aligned_used;
    arena->used = aligned_used + size;

    if (zero) {
        core::memset(result, 0, size);
    }

    return result;
}

inline void arena_pop_to(MemoryArena* arena, usize pos) noexcept {
    if (arena) {
        if (pos < arena->size) {
            arena->used = pos;
        } else {
            arena->used = arena->size;
        }
    }
}

inline void arena_clear(MemoryArena* arena) noexcept {
    arena_pop_to(arena, 0);
}

#define push_struct(arena, type) (type *)arena_push(arena, sizeof(type), alignof(type), true)
#define push_array(arena, count, type) (type *)arena_push(arena, (count) * sizeof(type), alignof(type), true)

#endif // CORE_MEMORY_ARENA_H
