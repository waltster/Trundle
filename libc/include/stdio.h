#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdint.h>

#define EOF (-1)
#define NULL ((void*)0)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char *);

#define kerror(msg) {printf("Kernel Error: %s\n", msg); }
#define panic(msg) { kerror(msg); abort(); }

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

#ifdef __cplusplus
}
#endif

#endif
