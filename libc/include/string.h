#ifndef _STRING_H
#define _STRING_H 1
 
#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
 
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
size_t strnlen(const char* str, size_t maxlen);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);
char *strncpy(char *, const char*, size_t);

#ifdef __cplusplus
}
#endif
 
#endif
