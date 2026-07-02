#include <common/types.h>
#include <stdarg.h>

using myos::common::size_t;

extern "C"
{
    void* memcpy(void* dst, const void* src, size_t n)
    {
        char* d = (char*)dst;
        const char* s = (const char*)src;
        while(n--)
            *d++ = *s++;
        return dst;
    }

    void* memset(void* dst, int c, size_t n)
    {
        char* d = (char*)dst;
        while(n--)
            *d++ = (char)c;
        return dst;
    }

    void* memmove(void* dst, const void* src, size_t n)
    {
        char* d = (char*)dst;
        const char* s = (const char*)src;
        if(d < s)
        {
            while(n--)
                *d++ = *s++;
        }
        else
        {
            d += n;
            s += n;
            while(n--)
                *--d = *--s;
        }
        return dst;
    }

    int memcmp(const void* a, const void* b, size_t n)
    {
        const char* p = (const char*)a;
        const char* q = (const char*)b;
        while(n--)
        {
            if(*p != *q)
                return *(unsigned char*)p - *(unsigned char*)q;
            p++;
            q++;
        }
        return 0;
    }

    size_t strlen(const char* s)
    {
        size_t len = 0;
        while(s[len])
            len++;
        return len;
    }

    int strcmp(const char* s1, const char* s2)
    {
        while(*s1 && (*s1 == *s2)) {
            s1++;
            s2++;
        }
        return *(unsigned char*)s1 - *(unsigned char*)s2;
    }

    int strncmp(const char* s1, const char* s2, size_t n)
    {
        while(n--) {
            if(*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2;
            if(*s1 == '\0') return 0;
            s1++;
            s2++;
        }
        return 0;
    }

    char* strncpy(char* dst, const char* src, size_t n)
    {
        char* d = dst;
        while(n > 0 && *src)
        {
            *d++ = *src++;
            n--;
        }
        while(n > 0)
        {
            *d++ = '\0';
            n--;
        }
        return dst;
    }

    static void _vsnprintf_putchar(char** p, char* end, char c)
    {
        if(*p < end) *(*p)++ = c;
    }

    static void _vsnprintf_putstr(char** p, char* end, const char* s, int maxlen)
    {
        while(maxlen != 0 && *s)
        {
            _vsnprintf_putchar(p, end, *s++);
            if(maxlen > 0) maxlen--;
        }
    }

    static void _vsnprintf_putnum(char** p, char* end, unsigned long long val,
                                   int base, int uppercase, int pad, int width)
    {
        const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
        char buf[65];
        int i = 0;
        if(val == 0) buf[i++] = '0';
        else while(val > 0)
        {
            buf[i++] = digits[val % base];
            val /= base;
        }
        while(i < pad) buf[i++] = '0';
        while(i > 0) _vsnprintf_putchar(p, end, buf[--i]);
        (void)width;
    }

    int vsnprintf(char *str, size_t size, const char *format, va_list ap)
    {
        char* p = str;
        char* end = str + (size > 0 ? size - 1 : 0);
        int count = 0;

        while(*format)
        {
            if(*format != '%')
            {
                _vsnprintf_putchar(&p, end, *format++);
                count++;
                continue;
            }
            format++;

            int width = -1;
            int prec = -1;
            int pad = ' ';

            if(*format == '0') { pad = '0'; format++; }

            if(*format >= '0' && *format <= '9')
            {
                width = 0;
                while(*format >= '0' && *format <= '9')
                    width = width * 10 + (*format++ - '0');
            }

            if(*format == '.')
            {
                format++;
                prec = 0;
                while(*format >= '0' && *format <= '9')
                    prec = prec * 10 + (*format++ - '0');
            }

            if(*format == 'l') format++;

            switch(*format)
            {
                case 's':
                {
                    const char* s = va_arg(ap, const char*);
                    _vsnprintf_putstr(&p, end, s, prec);
                    count += prec > 0 ? prec : (s ? strlen(s) : 0);
                    break;
                }
                case 'd':
                {
                    long long val = va_arg(ap, int);
                    if(val < 0)
                    {
                        _vsnprintf_putchar(&p, end, '-');
                        count++;
                        val = -val;
                    }
                    _vsnprintf_putnum(&p, end, val, 10, 0, pad == '0' ? width : 0, width);
                    count += 1;
                    break;
                }
                case 'u':
                {
                    unsigned long long val = va_arg(ap, unsigned int);
                    _vsnprintf_putnum(&p, end, val, 10, 0, 0, 0);
                    count += 1;
                    break;
                }
                case 'x':
                case 'X':
                {
                    unsigned long long val = va_arg(ap, unsigned int);
                    _vsnprintf_putnum(&p, end, val, 16, *format == 'X', 0, 0);
                    count += 1;
                    break;
                }
                case 'c':
                {
                    char c = (char)va_arg(ap, int);
                    _vsnprintf_putchar(&p, end, c);
                    count++;
                    break;
                }
                case '%':
                    _vsnprintf_putchar(&p, end, '%');
                    count++;
                    break;
                default:
                    _vsnprintf_putchar(&p, end, *format);
                    count++;
                    break;
            }
            format++;
        }

        if(size > 0) *p = '\0';
        return count;
    }
}
