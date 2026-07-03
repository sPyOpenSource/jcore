#ifndef __MYOS__STDIO_H
#define __MYOS__STDIO_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif

void printf(const char* str);

#endif
