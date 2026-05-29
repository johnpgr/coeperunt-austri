#include "path.h"

Path
path_parse(MemoryArena* arena, String8 input) noexcept {
    Path result = {};
    if (input.size == 0) {
        return result;
    }

    u8* p = input.str;
    u8* end = input.str + input.size;

    // 1. Detect header (Windows drive letter or protocol scheme)
    u8* colon = nullptr;
    for (u8* c = p; c < end; ++c) {
        if (*c == '/' || *c == '\\') {
            break;
        }
        if (*c == ':') {
            colon = c;
            break;
        }
    }

    if (colon) {
        b8 is_valid_header = false;
        if (p < colon) {
            // Windows drive letter: exactly 1 character A-Z or a-z
            if (colon - p == 1) {
                u8 drive = *p;
                if ((drive >= 'a' && drive <= 'z') || (drive >= 'A' && drive <= 'Z')) {
                    is_valid_header = true;
                }
            } else {
                // Protocol scheme: starts with a letter, followed by letters, digits, '+', '-', '.'
                u8 first = *p;
                if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')) {
                    is_valid_header = true;
                    for (u8* c = p + 1; c < colon; ++c) {
                        u8 ch = *c;
                        b8 is_valid_char = ((ch >= 'a' && ch <= 'z') ||
                                            (ch >= 'A' && ch <= 'Z') ||
                                            (ch >= '0' && ch <= '9') ||
                                            ch == '+' || ch == '-' || ch == '.');
                        if (!is_valid_char) {
                            is_valid_header = false;
                            break;
                        }
                    }
                }
            }
        }

        if (is_valid_header) {
            result.header_string.str = p;
            result.header_string.size = (usize)(colon - p + 1); // includes the colon
            p = colon + 1;
        }
    }

    // 2. Count leading slashes
    while (p < end && (*p == '/' || *p == '\\')) {
        result.number_of_leading_slashes++;
        p++;
    }

    // 3. Check for trailing slash
    u8* last_non_ws = end - 1;
    while (last_non_ws >= p && (*last_non_ws == ' ' || *last_non_ws == '\t' || *last_non_ws == '\r' || *last_non_ws == '\n')) {
        last_non_ws--;
    }
    if (last_non_ws >= p && (*last_non_ws == '/' || *last_non_ws == '\\')) {
        result.trailing_slash = true;
    }

    // 4. Parse components (words) - two-pass parser
    u8* scan = p;
    u32 estimated_word_count = 0;
    b8 in_word = false;
    while (scan <= last_non_ws) {
        u8 c = *scan;
        b8 is_slash = (c == '/' || c == '\\');
        if (is_slash) {
            if (in_word) {
                in_word = false;
            }
        } else {
            if (!in_word) {
                estimated_word_count++;
                in_word = true;
            }
        }
        scan++;
    }

    if (estimated_word_count > 0) {
        result.words = push_array(arena, estimated_word_count, String8);
        result.word_count = 0;

        scan = p;
        in_word = false;
        u8* word_start = nullptr;
        while (scan <= last_non_ws) {
            u8 c = *scan;
            b8 is_slash = (c == '/' || c == '\\');
            if (is_slash) {
                if (in_word) {
                    result.words[result.word_count++] = str8(word_start, (usize)(scan - word_start));
                    in_word = false;
                }
            } else {
                if (!in_word) {
                    word_start = scan;
                    in_word = true;
                }
            }
            scan++;
        }
        if (in_word) {
            result.words[result.word_count++] = str8(word_start, (usize)(last_non_ws + 1 - word_start));
        }
    }

    return result;
}

Path
path_copy(MemoryArena* arena, Path path) noexcept {
    Path result = {};

    if (path.header_string.size > 0) {
        result.header_string.str = (u8*)arena_push(arena, path.header_string.size, 1, false);
        core::memcpy(result.header_string.str, path.header_string.str, path.header_string.size);
        result.header_string.size = path.header_string.size;
    }

    result.number_of_leading_slashes = path.number_of_leading_slashes;
    result.trailing_slash = path.trailing_slash;
    result.word_count = path.word_count;

    if (path.word_count > 0) {
        result.words = push_array(arena, path.word_count, String8);
        for (u32 i = 0; i < path.word_count; ++i) {
            String8 word = path.words[i];
            result.words[i].str = (u8*)arena_push(arena, word.size, 1, false);
            core::memcpy(result.words[i].str, word.str, word.size);
            result.words[i].size = word.size;
        }
    }

    return result;
}

Path
path_reduce(MemoryArena* arena, Path path) noexcept {
    Path result = {};
    result.header_string = path.header_string;
    result.number_of_leading_slashes = path.number_of_leading_slashes;
    result.trailing_slash = path.trailing_slash;

    if (path.word_count == 0) {
        return result;
    }

    // Allocate worst-case array size on the arena
    String8* out_words = push_array(arena, path.word_count, String8);
    u32 out_count = 0;

    b8 is_absolute = (path.number_of_leading_slashes > 0 || path.header_string.size > 0);

    for (u32 i = 0; i < path.word_count; ++i) {
        String8 word = path.words[i];
        if (word.size == 1 && word.str[0] == '.') {
            // Ignore "."
            continue;
        }
        if (word.size == 2 && word.str[0] == '.' && word.str[1] == '.') {
            // It's ".."
            if (out_count > 0) {
                String8 top = out_words[out_count - 1];
                if (top.size == 2 && top.str[0] == '.' && top.str[1] == '.') {
                    // Top is also "..", keep building relative parent references
                    out_words[out_count++] = word;
                } else {
                    // Pop
                    out_count--;
                }
            } else {
                if (is_absolute) {
                    // In absolute paths, ".." at root is ignored
                    continue;
                } else {
                    // Relative path, keep leading ".."
                    out_words[out_count++] = word;
                }
            }
            continue;
        }

        // Regular component
        out_words[out_count++] = word;
    }

    result.words = out_words;
    result.word_count = out_count;
    return result;
}

String8
path_to_string(MemoryArena* arena, Path path, PathStyle style) noexcept {
    u8 sep = '/';
    if (style == PathStyle_Windows) {
        sep = '\\';
    } else if (style == PathStyle_Native) {
#if defined(OS_WINDOWS)
        sep = '\\';
#else
        sep = '/';
#endif
    }

    usize total_size = 0;
    total_size += path.header_string.size;
    total_size += path.number_of_leading_slashes;

    for (u32 i = 0; i < path.word_count; ++i) {
        total_size += path.words[i].size;
        if (i < path.word_count - 1) {
            total_size += 1;
        }
    }

    if (path.trailing_slash && path.word_count > 0) {
        total_size += 1;
    }

    if (total_size == 0) {
        return str8_zero();
    }

    u8* buf = (u8*)arena_push(arena, total_size + 1, 1, false); // +1 for null terminator compatibility
    if (!buf) {
        return str8_zero();
    }

    u8* dest = buf;

    if (path.header_string.size > 0) {
        core::memcpy(dest, path.header_string.str, path.header_string.size);
        dest += path.header_string.size;
    }

    for (i32 i = 0; i < path.number_of_leading_slashes; ++i) {
        *dest++ = sep;
    }

    for (u32 i = 0; i < path.word_count; ++i) {
        String8 word = path.words[i];
        core::memcpy(dest, word.str, word.size);
        dest += word.size;
        if (i < path.word_count - 1) {
            *dest++ = sep;
        }
    }

    if (path.trailing_slash && path.word_count > 0) {
        *dest++ = sep;
    }

    *dest = '\0'; // null terminator

    return str8(buf, total_size);
}

Path
path_join(MemoryArena* arena, Path base, Path tail) noexcept {
    // If tail is fully absolute (has drive letter/protocol)
    if (tail.header_string.size > 0) {
        return path_copy(arena, tail);
    }

    // If tail starts with leading slashes but no drive, e.g. "\foo"
    if (tail.number_of_leading_slashes > 0) {
        Path result = path_copy(arena, tail);
        if (base.header_string.size > 0) {
            result.header_string.str = (u8*)arena_push(arena, base.header_string.size, 1, false);
            core::memcpy(result.header_string.str, base.header_string.str, base.header_string.size);
            result.header_string.size = base.header_string.size;
        }
        return result;
    }

    // Both are relative, or tail is relative to base
    Path result = {};
    if (base.header_string.size > 0) {
        result.header_string.str = (u8*)arena_push(arena, base.header_string.size, 1, false);
        core::memcpy(result.header_string.str, base.header_string.str, base.header_string.size);
        result.header_string.size = base.header_string.size;
    }

    result.number_of_leading_slashes = base.number_of_leading_slashes;
    result.trailing_slash = tail.trailing_slash;
    result.word_count = base.word_count + tail.word_count;

    if (result.word_count > 0) {
        result.words = push_array(arena, result.word_count, String8);
        u32 idx = 0;
        for (u32 i = 0; i < base.word_count; ++i) {
            String8 word = base.words[i];
            result.words[idx].str = (u8*)arena_push(arena, word.size, 1, false);
            core::memcpy(result.words[idx].str, word.str, word.size);
            result.words[idx].size = word.size;
            idx++;
        }
        for (u32 i = 0; i < tail.word_count; ++i) {
            String8 word = tail.words[i];
            result.words[idx].str = (u8*)arena_push(arena, word.size, 1, false);
            core::memcpy(result.words[idx].str, word.str, word.size);
            result.words[idx].size = word.size;
            idx++;
        }
    }

    return result;
}
