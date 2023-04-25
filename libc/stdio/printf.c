#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define DO_DIVISION(n, base) ({ \
int __res; \
__asm__("divl %4" : "=a" (n), "=d" (__res) : "0" (n), "1" (0), "r" (base)); \
__res; })

#define FLAG_ZEROPAD    0x01
#define FLAG_SIGN       0x02
#define FLAG_PLUS       0x04
#define FLAG_SPACE      0x08
#define FLAG_LEFT       0x10
#define FLAG_SPECIAL    0x20
#define FLAG_SMALL      0x40
#define LOWERCASE_DIGITS "0123456789abcdefghijklmnopqrstuvwxyz"
#define UPPERCASE_DIGITS "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*
 * Perform a string to integer conversion
 *
 * @param s a pointer of strings
 */
static int skip_atoi(const char **s) {
    int i = 0;

    while (IS_DIGIT(**s)) {
        i = i * 10 + *((*s)++) - '0';
    }

    return i;
}

static char *_number(char *str, int num, int base, int size, int precision,
       int type) {
    char c;
    char sign;
    const char *digits = UPPERCASE_DIGITS;
    char temp[36];
    int i;

    if (type & FLAG_SMALL) {
        digits = LOWERCASE_DIGITS;
    } 

    if (type & FLAG_LEFT) {
        type &= ~FLAG_ZEROPAD;
    }

    if (base < 2 || base > 36) {
        return 0;
    }

    // If there is a need to zero-pad, do it
    c = (type & FLAG_ZEROPAD) ? '0' : ' ';

    // If there is a need to sign the digit
    if (type & FLAG_SIGN && num < 0) {
        sign = '-';
        num = -num;
    } else {
        sign = (type & FLAG_PLUS) ? '+' : ((type & FLAG_SPACE) ? ' ' : 0);
    }

    if (sign) {
        size--;
    }

    if (type & FLAG_SPECIAL) {
        if (base == 16) {
            size -= 2;
        } else if (base == 8) {
            size -= 1;
        }
    }

    i = 0;

    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num != 0) {
            temp[i++] = digits[DO_DIVISION(num, base)];
        }
    }

    if (i > precision) {
        precision = i;
    }

    size -= precision;

    if (!(type & (FLAG_ZEROPAD + FLAG_LEFT))) {
        while(size-- > 0) {
            *str++ = ' ';
        }
    }

    if (sign) {
        *str++ = sign;
    }

    if (type & FLAG_SPECIAL) {
        if (base == 8) {
            *str++ = '0';
        } else if (base == 16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }

    if (!(type & FLAG_LEFT)) {
        while (size-- > 0) {
            *str++ = c;
        }
    }

    while (i < precision--) {
        *str++ = c;
    }

    while (i-- > 0) {
        *str++ = temp[i];
    }

    while (size-- > 0){
        *str++ = ' ';
    }

    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    int len;
    int i;
    char *str;
    char *s;
    int *ip;
    int flags = 0;
    int field_width = -1;
    int precision;
    int qualifier;

    for (str = buf; *fmt; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        flags = 0;
        field_width = -1;
        precision = -1;
        qualifier = -1;

repeat:
        ++fmt;
        switch (*fmt) {
            case '-': 
                flags |= FLAG_LEFT;
                goto repeat;
            case '+':
                flags |= FLAG_PLUS;
                goto repeat;
            case ' ':
                flags |= FLAG_SPACE;
                goto repeat;
            case '#':
                flags |= FLAG_SPECIAL;
                goto repeat;
            case '0':
                flags |= FLAG_ZEROPAD;
                goto repeat;
        }

        if (IS_DIGIT(*fmt)) {
            field_width = skip_atoi(&fmt);
        } else if (*fmt == '*') {
            field_width = va_arg(args, int);

            if (field_width < 0) {
                field_width = -field_width;
                flags |= FLAG_LEFT;
            }
        }

        if (*fmt == '.') {
            ++fmt;

            if (IS_DIGIT(*fmt)) {
                precision = skip_atoi(&fmt);
            } else if (*fmt == '*') {
                precision = va_arg(args, int);
            }

            if (precision < 0) {
                precision = 0;
            }
        }

        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            ++fmt;
        }

        switch (*fmt) {
            case 'c':
                if (!(flags & FLAG_LEFT)) {
                    while (--field_width > 0) {
                        *str++ = ' ';
                    }
                }

                *str++ = (unsigned char) va_arg(args, int);

                while (--field_width> 0){
                    *str++ = ' ';
                }

                break;
            case 's':
                s = va_arg(args, char*);
                len = strlen(s);

                if (precision < 0) {
                    precision = len;
                } else if (len > precision) {
                    len = precision;
                }

                if (!(flags & FLAG_LEFT)) {
                    while (len < field_width--) {
                        *str++ = ' ';
                    }
                }

                for (i = 0; i < len; ++i) {
                    *str++ = *s++;
                }

                while (len < field_width--) {
                    *str++ = ' ';
                }

                break;
            case 'o':
                str = _number(str, va_arg(args, unsigned long), 8, field_width,
                        precision, flags);
                break;
            case 'p':
                if (field_width == -1) {
                    field_width = 8;
                    flags |= FLAG_ZEROPAD;
                }

                str = _number(str, (unsigned long)va_arg(args, void*), 16, 
                        field_width, precision, flags);
                break;
            case 'x':
                flags |= FLAG_SMALL;
            case 'X':
                str = _number(str, va_arg(args, unsigned long), 16, 
                        field_width, precision, flags);
                break;
            case 'd':
            case 'i':
                flags |= FLAG_SIGN;
            case 'u':
                str = _number(str, va_arg(args, unsigned long), 10,
                        field_width, precision, flags);
                break;
            case 'b':
                str = _number(str, va_arg(args, unsigned long), 2,
                        field_width, precision, flags);
                break;
            case 'n':
                ip = va_arg(args, int*);
                *ip = (str - buf);
                break;
            default:
                if (*fmt != '%') {
                    *str++ = '%';
                }

                if (*fmt) {
                    *str++ = *fmt;
                } else {
                    fmt--;
                }

                break;
        }
    }

    *str = '\0';

    return str - buf;
}

int printf(const char *fmt, ...) {
    static char buf[2048];
    va_list args;
    int i;
    int j;

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);

    buf[i] = '\0';
    
    for (j = 0; j < (int)strnlen(buf, 2048); j++) {
        putchar(buf[j]);
    }

    return j;
}

