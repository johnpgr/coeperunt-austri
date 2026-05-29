#include "fs.h"

#include "../win32.h"

String8 win32_read_entire_file(MemoryArena* arena, Path filepath) noexcept {
    String8 result = {};
    if (filepath.word_count == 0 && filepath.number_of_leading_slashes == 0 && filepath.header_string.size == 0) {
        return result;
    }

    u8 stack_buf[512];
    MemoryArena stack_arena = {};
    stack_arena.base = stack_buf;
    stack_arena.size = sizeof(stack_buf);
    
    String8 win32_path = path_to_string(&stack_arena, filepath, PathStyle_Windows);
    if (win32_path.size == 0 || win32_path.size >= sizeof(stack_buf)) {
        return result;
    }

    char resolved_path[512];
    core::memset(resolved_path, 0, sizeof(resolved_path));

    b8 is_absolute = (filepath.number_of_leading_slashes > 0 || filepath.header_string.size > 0);

    if (is_absolute) {
        core::memcpy(resolved_path, win32_path.str, win32_path.size);
    } else {
        DWORD base_len = GetModuleFileNameA(nullptr, resolved_path, sizeof(resolved_path));
        if (base_len > 0) {
            usize last_slash = 0;
            for (usize i = (usize)base_len; i > 0; --i) {
                if (resolved_path[i - 1] == '\\' || resolved_path[i - 1] == '/') {
                    last_slash = i - 1;
                    break;
                }
            }
            resolved_path[last_slash + 1] = '\0';
            
            usize dir_len = core::strlen(resolved_path);
            if (dir_len + win32_path.size < sizeof(resolved_path)) {
                core::memcpy(resolved_path + dir_len, win32_path.str, win32_path.size);
                resolved_path[dir_len + win32_path.size] = '\0';
            }
        } else {
            core::memcpy(resolved_path, win32_path.str, win32_path.size);
        }
    }

    HANDLE file = CreateFileA(
        resolved_path,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (file == INVALID_HANDLE_VALUE) {
        core::printf("[Error] Failed to open file: %s\n", resolved_path);
        return result;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(file, &size)) {
        core::printf("[Error] Failed to query file size: %s\n", resolved_path);
        CloseHandle(file);
        return result;
    }

    if (size.QuadPart <= 0) {
        CloseHandle(file);
        return result;
    }

    if ((u64)size.QuadPart > (u64)(usize)-1) {
        core::printf("[Error] File size overflow: %s\n", resolved_path);
        CloseHandle(file);
        return result;
    }

    usize file_size = (usize)size.QuadPart;
    void* data = arena_push(arena, file_size);
    if (!data) {
        core::printf("[Error] Failed to allocate file memory on arena for: %s\n", resolved_path);
        CloseHandle(file);
        return result;
    }

    u8* dest = (u8*)data;
    usize remaining = file_size;
    while (remaining > 0) {
        DWORD to_read = (remaining > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (DWORD)remaining;
        DWORD bytes_read = 0;
        b8 ok = ReadFile(file, dest, to_read, &bytes_read, nullptr);
        if (!ok || bytes_read == 0) {
            core::printf("[Error] Failed to read file content: %s\n", resolved_path);
            break;
        }
        dest += bytes_read;
        remaining -= bytes_read;
    }

    CloseHandle(file);
    if (remaining != 0) {
        return result;
    }

    result.str = (u8*)data;
    result.size = file_size;
    return result;
}

b8 win32_write_entire_file(
    const char* filepath,
    const void* data,
    usize size
) noexcept {
    if (!filepath || (!data && size > 0)) {
        return false;
    }

    char resolved_path[512];
    core::memset(resolved_path, 0, sizeof(resolved_path));

    b8 is_absolute = false;
    if (filepath[0] && filepath[1] == ':') {
        is_absolute = true;
    } else if (filepath[0] == '\\' && filepath[1] == '\\') {
        is_absolute = true;
    } else if (filepath[0] == '/') {
        is_absolute = true;
    }

    if (is_absolute) {
        usize file_len = core::strlen(filepath);
        if (file_len < sizeof(resolved_path)) {
            core::memcpy(resolved_path, filepath, file_len);
        }
    } else {
        DWORD base_len = GetModuleFileNameA(nullptr, resolved_path, sizeof(resolved_path));
        if (base_len > 0) {
            usize last_slash = 0;
            for (usize i = (usize)base_len; i > 0; --i) {
                if (resolved_path[i - 1] == '\\' || resolved_path[i - 1] == '/') {
                    last_slash = i - 1;
                    break;
                }
            }
            resolved_path[last_slash + 1] = '\0';
            
            usize dir_len = core::strlen(resolved_path);
            usize file_len = core::strlen(filepath);
            if (dir_len + file_len < sizeof(resolved_path)) {
                core::memcpy(resolved_path + dir_len, filepath, file_len);
                resolved_path[dir_len + file_len] = '\0';
            }
        } else {
            usize file_len = core::strlen(filepath);
            if (file_len < sizeof(resolved_path)) {
                core::memcpy(resolved_path, filepath, file_len);
            }
        }
    }

    HANDLE file = CreateFileA(
        resolved_path,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (file == INVALID_HANDLE_VALUE) {
        core::printf("[Error] Failed to open file for writing: %s (resolved: %s)\n", filepath, resolved_path);
        return false;
    }

    usize total_written = 0;
    const u8* src       = (const u8*)data;
    b8 success          = true;
    while (total_written < size) {
        usize remaining = size - total_written;
        DWORD to_write =
            (remaining > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (DWORD)remaining;

        DWORD bytes_written = 0;
        if (!WriteFile(
                file,
                src + total_written,
                to_write,
                &bytes_written,
                nullptr
            ) ||
            bytes_written == 0) {
            core::printf(
                "[Error] Failed to write file content: %s\n",
                filepath
            );
            success = false;
            break;
        }
        total_written += bytes_written;
    }

    if (success) {
        success = (total_written == size);
    }

    CloseHandle(file);
    return success;
}
