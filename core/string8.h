#ifndef CORE_STRING8_H
#define CORE_STRING8_H

#include "../core.h"

struct String8 {
    u8* str;
    usize size;
};

#define str8_lit(S) String8{(u8*)(S), sizeof(S) - 1}
#define str8_varg(S) (int)((S).size), (const char*)((S).str)

inline String8
str8(u8* str, usize size) noexcept {
    String8 result;
    result.str = str;
    result.size = size;
    return result;
}

inline String8
str8_cstring(const char* cstr) noexcept {
    String8 result;
    result.str = (u8*)cstr;
    result.size = cstr ? core::strlen(cstr) : 0;
    return result;
}

inline String8
str8_zero() noexcept {
    String8 result = {};
    return result;
}

inline b32
str8_match(String8 a, String8 b) noexcept {
    if (a.size != b.size) {
        return false;
    }
    for (usize i = 0; i < a.size; ++i) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
    }
    return true;
}

inline String8
str8_substr(String8 str, usize start, usize length) noexcept {
    if (start >= str.size) {
        return str8_zero();
    }
    usize available = str.size - start;
    usize actual_length = (length < available) ? length : available;
    
    String8 result;
    result.str = str.str + start;
    result.size = actual_length;
    return result;
}

#endif // CORE_STRING8_H
