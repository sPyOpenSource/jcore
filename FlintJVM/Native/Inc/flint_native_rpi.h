
#ifndef __FLINT_NATIVE_RPI_H
#define __FLINT_NATIVE_RPI_H

#include "flint_native.h"

jvoid NativeRPi_Print(FNIEnv *env, jstring msg);
jlong NativeRPi_CurrentTimeMillis(FNIEnv *env);
jvoid NativeRPi_SetPin(FNIEnv *env, jint pin, jint value);
jint NativeRPi_Random(FNIEnv *env);

static const NativeMethod rpiMethods[] = {
    NATIVE_METHOD("rpiPrint",          "(Ljava/lang/String;)V", NativeRPi_Print),
    NATIVE_METHOD("rpiCurrentTimeMillis", "()J",                NativeRPi_CurrentTimeMillis),
    NATIVE_METHOD("rpiSetPin",         "(II)V",                 NativeRPi_SetPin),
    NATIVE_METHOD("rpiRandom",         "()I",                   NativeRPi_Random),
};

#endif /* __FLINT_NATIVE_RPI_H */
