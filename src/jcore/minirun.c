#include "all.h"
#include <stdarg.h>

extern void putchar(int c);

int dprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p == '%') {
            p++;
            if (*p == 'd') {
                int d = va_arg(args, int);
                if (d < 0) {
                    putchar('-');
                    d = -d;
                }
                if (d == 0) {
                    putchar('0');
                } else {
                    char buf[12];
                    int i = 0;
                    while (d > 0) {
                        buf[i++] = (d % 10) + '0';
                        d /= 10;
                    }
                    while (i > 0) putchar(buf[--i]);
                }
            } else if (*p == 'x') {
                unsigned int x = va_arg(args, unsigned int);
                const char *hex = "0123456789abcdef";
                if (x == 0) {
                    putchar('0');
                } else {
                    char buf[12];
                    int i = 0;
                    while (x > 0) {
                        buf[i++] = hex[x % 16];
                        x /= 16;
                    }
                    while (i > 0) putchar(buf[--i]);
                }
            } else if (*p == 'p') {
                void *ptr = va_arg(args, void *);
                dprintf("0x%x", (unsigned int)ptr);
            } else if (*p == 's') {
                const char *s = va_arg(args, const char *);
                while (s && *s) putchar(*s++);
            } else {
                putchar(*p);
            }
        } else {
            putchar(*p);
        }
    }
    va_end(args);
    return 0;
}

void sys_panic(char *msg, ...) {
    dprintf("\n!!! PANIC: ");
    va_list args;
    va_start(args, msg);
    
    for (const char *p = msg; *p != '\0'; p++) {
        if (*p == '%') {
            p++;
            if (*p == 'd') {
                int d = va_arg(args, int);
                dprintf("%d", d);
            } else if (*p == 's') {
                const char *s = va_arg(args, const char *);
                while (s && *s) putchar(*s++);
            } else {
                putchar(*p);
            }
        } else {
            putchar(*p);
        }
    }
    va_end(args);
    dprintf(" !!!\n");
    while (1) {
        asm volatile("wfi");
    }
}