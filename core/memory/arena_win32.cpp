#include "arena.h"
#include "../../platform/win32.h"

MemoryArena arena_alloc(usize size) noexcept {
    MemoryArena arena = {};
    if (size == 0) {
        return arena;
    }

    void* base = VirtualAlloc(
        nullptr,
        size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
    if (base) {
        arena.base = (u8*)base;
        arena.size = size;
        arena.used = 0;
    }
    return arena;
}

void arena_release(MemoryArena* arena) noexcept {
    if (arena && arena->base) {
        VirtualFree(arena->base, 0, MEM_RELEASE);
        arena->base = nullptr;
        arena->size = 0;
        arena->used = 0;
    }
}
