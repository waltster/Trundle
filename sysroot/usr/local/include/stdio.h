#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdlib.h>

#define EOF (-1)
#define NULL ((void*)0)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char *);

#define kerror(msg) {printf("Kernel Error: %s (%s:%d)\n", msg, \
        __FILE__, __LINE__); }
#define panic(msg) { kerror(msg); abort(); }
#define assert(cond) { if (!(cond)) { \
    printf("Kernel Error: Assertion failed %s (%s: %d)\n", \
    #cond, __FILE__, __LINE__); abort(); \
} }

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

#ifdef __cplusplus
}
#endif

#endif
