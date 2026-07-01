#include <jvm/jvm_bridge.h>
#include <drivers/vga.h> // Or wherever printf is
#include <memorymanagement.h>

namespace myos {
    namespace jvm {
        void JVMBridge::Initialize() {
            // Basic init
        }

        void* JVMBridge::AllocateMemory(size_t size) {
            return MemoryManager::activeMemoryManager->malloc(size);
        }

        void JVMBridge::FreeMemory(void* ptr) {
            MemoryManager::activeMemoryManager->free(ptr);
        }

        void JVMBridge::Println(const char* str) {
            // To be implemented: call printf
        }
    }
}
