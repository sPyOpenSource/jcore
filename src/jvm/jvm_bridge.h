#ifndef __MYOS__JVM_BRIDGE_H
#define __MYOS__JVM_BRIDGE_H

#include <common/types.h>

namespace myos {
    namespace jvm {
        class JVMBridge {
        public:
            static void Initialize();
            static void* AllocateMemory(size_t size);
            static void FreeMemory(void* ptr);
            static void Println(const char* str);
        };
    }
}

#endif
