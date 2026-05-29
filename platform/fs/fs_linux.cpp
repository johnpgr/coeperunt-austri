#include "fs.h"

String8 linux_read_entire_file(MemoryArena* arena, Path filepath) noexcept {
    String8 result = {};
    if (filepath.word_count == 0 && filepath.number_of_leading_slashes == 0 && filepath.header_string.size == 0) {
        return result;
    }

    u8 stack_buf[512];
    MemoryArena stack_arena = {};
    stack_arena.base = stack_buf;
    stack_arena.size = sizeof(stack_buf);
    
    String8 unix_path = path_to_string(&stack_arena, filepath, PathStyle_Unix);
    if (unix_path.size == 0 || unix_path.size >= sizeof(stack_buf)) {
        return result;
    }

    i64 fd = __syscall4(
        SYS_OPENAT,
        LINUX_AT_FDCWD,
        (i64)unix_path.str,
        LINUX_O_RDONLY,
        0
    );
    if (fd < 0) {
        core::printf("[Error] Failed to open file: %s\n", (const char*)unix_path.str);
        return result;
    }

    i64 file_size = __syscall3(SYS_LSEEK, fd, 0, LINUX_SEEK_END);
    if (file_size <= 0) {
        __syscall1(SYS_CLOSE, fd);
        return result;
    }

    if (__syscall3(SYS_LSEEK, fd, 0, LINUX_SEEK_SET) < 0) {
        core::printf("[Error] Failed to seek file: %s\n", (const char*)unix_path.str);
        __syscall1(SYS_CLOSE, fd);
        return result;
    }

    if ((u64)file_size > (u64)(usize)-1) {
        core::printf("[Error] File size overflow: %s\n", (const char*)unix_path.str);
        __syscall1(SYS_CLOSE, fd);
        return result;
    }

    usize size = (usize)file_size;
    void* data = arena_push(arena, size);
    if (!data) {
        core::printf("[Error] Failed to allocate file memory on arena for: %s\n", (const char*)unix_path.str);
        __syscall1(SYS_CLOSE, fd);
        return result;
    }

    u8* dest = (u8*)data;
    usize remaining = size;
    while (remaining > 0) {
        i64 read_count = __syscall3(SYS_READ, fd, (i64)dest, (i64)remaining);
        if (read_count <= 0) {
            core::printf("[Error] Failed to read file content: %s\n", (const char*)unix_path.str);
            break;
        }
        dest += (usize)read_count;
        remaining -= (usize)read_count;
    }

    __syscall1(SYS_CLOSE, fd);
    if (remaining != 0) {
        return result;
    }

    result.str = (u8*)data;
    result.size = size;
    return result;
}

b8 linux_write_entire_file(
    const char* filepath,
    const void* data,
    usize size
) noexcept {
    if (!filepath || (!data && size > 0)) {
        return false;
    }

    i64 fd = __syscall4(
        SYS_OPENAT,
        LINUX_AT_FDCWD,
        (i64)filepath,
        LINUX_O_WRONLY | LINUX_O_CREAT | LINUX_O_TRUNC,
        0644
    );

    if (fd < 0) {
        core::printf("[Error] Failed to open file for writing: %s\n", filepath);
        return false;
    }

    usize total_written = 0;
    const u8* src       = (const u8*)data;
    while (total_written < size) {
        i64 write_count = __syscall3(
            SYS_WRITE,
            fd,
            (i64)(src + total_written),
            (i64)(size - total_written)
        );

        if (write_count <= 0) {
            core::printf(
                "[Error] Failed to write file content: %s\n",
                filepath
            );
            break;
        }

        total_written += (usize)write_count;
    }

    __syscall1(SYS_CLOSE, fd);
    return (total_written == size);
}
