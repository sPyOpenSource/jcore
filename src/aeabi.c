// Minimal ARM AEABI runtime helpers for baremetal (-nostdlib)
// These are called by compiler-generated code for memcpy/memset.

void __aeabi_memclr(void *dest, int n) {
    for (int i = 0; i < n; i++) ((char *)dest)[i] = 0;
}
void __aeabi_memclr4(void *d, int n) { __aeabi_memclr(d, n); }
void __aeabi_memclr8(void *d, int n) { __aeabi_memclr(d, n); }

void __aeabi_memcpy(void *dest, const void *src, int n) {
    for (int i = 0; i < n; i++) ((char *)dest)[i] = ((const char *)src)[i];
}
void __aeabi_memcpy4(void *d, const void *s, int n) { __aeabi_memcpy(d, s, n); }
void __aeabi_memcpy8(void *d, const void *s, int n) { __aeabi_memcpy(d, s, n); }

void __aeabi_memmove(void *dest, const void *src, int n) {
    char *d = dest;
    const char *s = src;
    if (d < s) {
        for (int i = 0; i < n; i++) d[i] = s[i];
    } else {
        for (int i = n - 1; i >= 0; i--) d[i] = s[i];
    }
}
void __aeabi_memmove4(void *d, const void *s, int n) { __aeabi_memmove(d, s, n); }
void __aeabi_memmove8(void *d, const void *s, int n) { __aeabi_memmove(d, s, n); }

void __aeabi_memset(void *dest, int n, int c) {
    for (int i = 0; i < n; i++) ((char *)dest)[i] = c;
}
void __aeabi_memset4(void *d, int n, int c) { __aeabi_memset(d, n, c); }
void __aeabi_memset8(void *d, int n, int c) { __aeabi_memset(d, n, c); }
