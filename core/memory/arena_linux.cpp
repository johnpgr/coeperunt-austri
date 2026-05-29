#include "arena.h"

MemoryArena arena_alloc(usize size) noexcept {
    MemoryArena arena = {};
    if (size == 0) {
        return arena;
    }

    void* base = sys::mmap(
        nullptr,
        size,
        LINUX_PROT_READ | LINUX_PROT_WRITE,
        LINUX_MAP_PRIVATE | LINUX_MAP_ANONYMOUS,
        -1,
        0
    );

    if (base != LINUX_MAP_FAILED) {
        arena.base = (u8*)base;
        arena.size = size;
        arena.used = 0;
    }
    return arena;
}

void arena_release(MemoryArena* arena) noexcept {
    if (arena && arena->base && arena->size > 0) {
        sys::munmap(arena->base, arena->size);
        arena->base = nullptr;
        arena->size = 0;
        arena->used = 0;
    }
}
