#ifndef __MYOS__STRING_H
#define __MYOS__STRING_H

#include <stdint.h>
#include <stddef.h>
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
void memcpy(void *dest, const void *src, size_t n);
void memmove(void *dest, const void *src, size_t n);
void memset(void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif

#endif
