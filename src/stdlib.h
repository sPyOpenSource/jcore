#ifndef __MYOS__STDLIB_H
#define __MYOS__STDLIB_H

#include <stdint.h>
#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

#endif
