extern "C"
{
    void* memcpy(void* dst, const void* src, unsigned int n)
    {
        char* d = (char*)dst;
        const char* s = (const char*)src;
        while(n--)
            *d++ = *s++;
        return dst;
    }

    void* memset(void* dst, int c, unsigned int n)
    {
        char* d = (char*)dst;
        while(n--)
            *d++ = (char)c;
        return dst;
    }

    void* memmove(void* dst, const void* src, unsigned int n)
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

    int memcmp(const void* a, const void* b, unsigned int n)
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

    unsigned int strlen(const char* s)
    {
        unsigned int len = 0;
        while(s[len])
            len++;
        return len;
    }

    char* strncpy(char* dst, const char* src, unsigned int n)
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
