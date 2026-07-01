
#include "flint.h"
#include "flint_system_api.h"
#include <string.h>

extern "C" {
    void rpi_uart_puts(const char *s);
    uint64_t rpi_get_ticks(void);
    int vsnprintf(char *str, size_t size, const char *format, va_list ap);
}

static void *heap_start = nullptr;
static void *heap_end = nullptr;
static void *heap_current = nullptr;

void FlintAPI::System::reset(void) {
    heap_current = heap_start;
}

void FlintAPI::System::initHeap(void *start, void *end) {
    heap_start = start;
    heap_end = end;
    heap_current = start;
}

void *FlintAPI::System::malloc(uint32_t size) {
    void *p = heap_current;
    if (p == nullptr) return nullptr;
    uintptr_t aligned = (((uintptr_t)p + 15) & ~15);
    p = (void *)aligned;
    heap_current = (void *)(aligned + size);
    if (heap_current > heap_end) return nullptr;
    return p;
}

void *FlintAPI::System::realloc(void *p, uint32_t size) {
    void *newp = malloc(size);
    if (newp && p) {
        memcpy(newp, p, size);
    }
    return newp;
}

void FlintAPI::System::free(void *p) {
    (void)p;
}

void FlintAPI::System::consoleWrite(uint8_t *utf8, uint32_t length) {
    char buf[256];
    uint32_t len = length < 255 ? length : 255;
    memcpy(buf, utf8, len);
    buf[len] = 0;
    rpi_uart_puts(buf);
}

int64_t FlintAPI::System::getTimeNanos(void) {
    return (int64_t)rpi_get_ticks() * 1000 / 54;
}

int64_t FlintAPI::System::getTimeMillis(void) {
    return (int64_t)rpi_get_ticks() / 54000;
}

JNMPtr FlintAPI::System::findNativeMethod(MethodInfo *methodInfo) {
    return nullptr;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    (void)str; (void)size; (void)format; (void)ap;
    return 0;
}
