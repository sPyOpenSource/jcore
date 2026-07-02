#include <stddef.h>
#include <flint_native_rpi.h>

extern "C" void __cxa_pure_virtual() {}

void operator delete(void* ptr, size_t) noexcept {
    (void)ptr;
}

// NativeRPi stubs (Raspberry Pi platform functions not used on x86)
void NativeRPi_Print(FNIEnv *env, jstring msg) { (void)env; (void)msg; }
jlong NativeRPi_CurrentTimeMillis(FNIEnv *env) { (void)env; return 0; }
void NativeRPi_SetPin(FNIEnv *env, jint pin, jint value) { (void)env; (void)pin; (void)value; }
jint NativeRPi_Random(FNIEnv *env) { (void)env; return 0; }
