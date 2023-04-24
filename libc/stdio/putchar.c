#include <stdio.h>

#ifdef __is_libk
#include <kernel/tty.h>
#endif

int putchar(int int_c) {
#ifdef __is_libk
    char c = (char)int_c;
    terminal_write(&c, sizeof(c));
#else
    // TODO: Implement syscall for writing
#endif
    return int_c;
}

