// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://codeberg.org/ThatOSDev/EFI_AARCH64

#ifndef C_LIB_H
#define C_LIB_H

#include <stddef.h>  // size_t

size_t strlen(const char* str);

int strcmp(const char* a, const char* b);

void itoa(uint64_t n, uint64_t* buffer, uint32_t basenumber);

void* memcpy(void* destination, const void* source, size_t num);

// GCC Version
void *memset (void *dest, register int val, register size_t len);

#endif  // C_LIB_H
