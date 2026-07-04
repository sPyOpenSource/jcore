#include <stddef.h>
#include <flint_native_rpi.h>
#include <flint_java_string.h>
#include <stdio.h>
#include <string.h>

extern "C" void __cxa_pure_virtual() {}

void operator delete(void* ptr, size_t) noexcept {
    (void)ptr;
}

// NativeRPi stubs — forward output to kernel serial console
void NativeRPi_Print(FNIEnv *env, jstring msg) { 
    (void)env; 
    if (msg == NULL) return;
    uint32_t len = msg->getLength();
    const char *str = msg->getAscii();
    if (str && len > 0) {
        char buf[1024];
        uint32_t n = len < 1023 ? len : 1023;
        memcpy(buf, str, n);
        buf[n] = '\0';
        printf(buf);
    }
}
jlong NativeRPi_CurrentTimeMillis(FNIEnv *env) { (void)env; return 0; }
void NativeRPi_SetPin(FNIEnv *env, jint pin, jint value) { (void)env; (void)pin; (void)value; }
jint NativeRPi_Random(FNIEnv *env) { (void)env; return 0; }

// x86os/Hello native methods
void MyCustomHelloPrint(FNIEnv *env, jstring msg) { 
    (void)env; 
    if (msg == NULL) return;
    uint32_t len = msg->getLength();
    const char *str = msg->getAscii();
    if (str && len > 0) {
        char buf[1024];
        uint32_t n = len < 1023 ? len : 1023;
        memcpy(buf, str, n);
        buf[n] = '\0';
        printf(buf);
    }
}
