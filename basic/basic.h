#ifndef BASIC_H
#define BASIC_H

#include "../core.h"

// Define freestanding va_list macros
#ifndef va_list
typedef __builtin_va_list va_list;
#endif
#ifndef va_start
#define va_start(ap, last) __builtin_va_start(ap, last)
#endif
#ifndef va_arg
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#endif
#ifndef va_copy
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#endif
#ifndef va_end
#define va_end(ap) __builtin_va_end(ap)
#endif

typedef struct Basic {
    void (*print)(const char* message) NOEXCEPT;
    void (*printf)(const char* format, ...) NOEXCEPT;
    void (*printv)(const char* format, va_list args) NOEXCEPT;
    void (*exit)(i32 status) NOEXCEPT;
} Basic;

EXTERN_C void basic_init(Basic* basic) NOEXCEPT;

// ============================================================================
// Freestanding custom_vsnprintf Formatter Helper
// ============================================================================
inline i32 custom_vsnprintf(char* buf, usize buf_size, const char* format, va_list args) NOEXCEPT {
    if (!buf || buf_size == 0) return 0;
    
    char* p = buf;
    char* end = buf + buf_size;
    
    while (p < end - 1 && *format) {
        if (*format == '%' && *(format + 1) != '\0') {
            format++; // skip '%'
            char spec = *format++;
            
            if (spec == '%') {
                *p++ = '%';
                continue;
            }
            
            if (spec == 's') {
                const char* val = va_arg(args, const char*);
                if (!val) val = "(null)";
                while (p < end - 1 && *val) {
                    *p++ = *val++;
                }
            } else if (spec == 'd' || spec == 'i') {
                i64 val = va_arg(args, int);
                char tmp[32];
                i32 i = 0;
                b8 neg = false;
                if (val < 0) {
                    neg = true;
                    u64 uval = (u64)-val;
                    do {
                        tmp[i++] = '0' + (uval % 10);
                        uval /= 10;
                    } while (uval > 0 && i < 32);
                } else {
                    u64 uval = (u64)val;
                    do {
                        tmp[i++] = '0' + (uval % 10);
                        uval /= 10;
                    } while (uval > 0 && i < 32);
                }
                if (neg && p < end - 1) {
                    *p++ = '-';
                }
                while (i > 0 && p < end - 1) {
                    *p++ = tmp[--i];
                }
            } else if (spec == 'u') {
                u64 val = va_arg(args, unsigned int);
                char tmp[32];
                i32 i = 0;
                do {
                    tmp[i++] = '0' + (val % 10);
                    val /= 10;
                } while (val > 0 && i < 32);
                while (i > 0 && p < end - 1) {
                    *p++ = tmp[--i];
                }
            } else if (spec == 'x' || spec == 'p') {
                u64 val;
                if (spec == 'p') {
                    val = (u64)(usize)va_arg(args, void*);
                } else {
                    val = va_arg(args, unsigned int);
                }
                char tmp[32];
                i32 i = 0;
                const char* digits = "0123456789abcdef";
                do {
                    tmp[i++] = digits[val % 16];
                    val /= 16;
                } while (val > 0 && i < 32);
                while (i > 0 && p < end - 1) {
                    *p++ = tmp[--i];
                }
            } else if (spec == 'c') {
                char val = (char)va_arg(args, int);
                *p++ = val;
            } else if (spec == 'f') {
                f64 val = va_arg(args, double);
                if (val < 0) {
                    if (p < end - 1) *p++ = '-';
                    val = -val;
                }
                u64 ipart = (u64)val;
                
                char tmp_i[32];
                i32 idx = 0;
                u64 ip = ipart;
                do {
                    tmp_i[idx++] = '0' + (ip % 10);
                    ip /= 10;
                } while (ip > 0 && idx < 32);
                while (idx > 0 && p < end - 1) {
                    *p++ = tmp_i[--idx];
                }
                
                if (p < end - 1) {
                    *p++ = '.';
                }
                
                f64 fpart = val - (f64)ipart;
                u64 fpart_val = (u64)(fpart * 1000000.0 + 0.5);
                char tmp[6];
                for (i32 i = 5; i >= 0; --i) {
                    tmp[i] = '0' + (fpart_val % 10);
                    fpart_val /= 10;
                }
                for (i32 i = 0; i < 6; ++i) {
                    if (p < end - 1) {
                        *p++ = tmp[i];
                    }
                }
            } else {
                if (p < end - 2) {
                    *p++ = '%';
                    *p++ = spec;
                }
            }
        } else {
            *p++ = *format++;
        }
    }
    *p = '\0';
    return (i32)(p - buf);
}

#endif // BASIC_H
