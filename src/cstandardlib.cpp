#include <common/types.h>

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
}
