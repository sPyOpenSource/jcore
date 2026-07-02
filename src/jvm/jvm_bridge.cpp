#include <jvm/jvm_bridge.h>
#include <drivers/vga.h>
#include <memorymanagement.h>
#include <multitasking.h>
#include <gdt.h>
#include <flint_system_api.h>
#include <jvm/embedded_classes.h>
#include <stdio.h>
#include <string.h>

namespace myos {
    namespace jvm {
        void JVMBridge::Initialize() {
            printf("JVM Bridge initialized.\n");
        }

        void* JVMBridge::AllocateMemory(myos::common::size_t size) {
            return MemoryManager::activeMemoryManager->malloc(size);
        }

        void JVMBridge::FreeMemory(void* ptr) {
            MemoryManager::activeMemoryManager->free(ptr);
        }

        void JVMBridge::Println(const char* str) {
            printf((char*)str);
            printf("\n");
        }
    }
}

namespace FlintAPI {
    namespace System {
        void reset(void) {}
        void *malloc(uint32_t size) { return myos::jvm::JVMBridge::AllocateMemory(size); }
        void *realloc(void *p, uint32_t size) { return myos::jvm::JVMBridge::AllocateMemory(size); } // Simplified
        void free(void *p) { myos::jvm::JVMBridge::FreeMemory(p); }
        void consoleWrite(uint8_t *utf8, uint32_t length) {
            char buf[1024];
            uint32_t len = length < 1023 ? length : 1023;
            memcpy(buf, utf8, len);
            buf[len] = '\0';
            printf(buf);
        }
        int64_t getTimeNanos(void) { return 0; }
        int64_t getTimeMillis(void) { return 0; }
        JNMPtr findNativeMethod(MethodInfo *methodInfo) { return nullptr; }
        void initHeap(void *start, void *end) {}
    }

    namespace Thread {
        typedef void * ThreadHandle;

        ThreadHandle create(void (*task)(void *), void *param, uint32_t stackSize) {
            using namespace myos;
            if (!GlobalDescriptorTable::activeGDT || !TaskManager::activeTaskManager) return nullptr;

            Task *t = new Task(GlobalDescriptorTable::activeGDT, task, param);
            if (TaskManager::activeTaskManager->AddTask(t)) {
                return (ThreadHandle)t;
            }
            delete t;
            return nullptr;
        }

        ThreadHandle getCurrentThread(void) { return nullptr; }
        void terminate(ThreadHandle handle) {}
        void sleep(uint32_t ms) {}
        void yield(void) {
            // In a real kernel, this would trigger a reschedule.
            // For the POC, we can leave it as a no-op or call a reschedule if available.
        }
    }
}

namespace FlintAPI {
    namespace IO {
        struct MemoryFile {
            const uint8_t *data;
            uint32_t size;
            uint32_t pos;
            bool isMemoryFile;
        };

        FileResult finfo(const char *fileName, FileInfo *fileInfo) {
            if (strcmp(fileName, "HelloWorld") == 0) {
                if (fileInfo) {
                    fileInfo->size = HELLO_WORLD_SIZE;
                }
                return FILE_RESULT_OK;
            }
            return FILE_RESULT_NO_PATH;
        }

        FileHandle fopen(const char *fileName, FileMode mode) {
            if (strcmp(fileName, "HelloWorld") == 0) {
                MemoryFile *mf = (MemoryFile *)myos::jvm::JVMBridge::AllocateMemory(sizeof(MemoryFile));
                mf->data = HELLO_WORLD_CLASS;
                mf->size = HELLO_WORLD_SIZE;
                mf->pos = 0;
                mf->isMemoryFile = true;
                return (FileHandle)mf;
            }
            return NULL;
        }

        FileResult fread(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
            MemoryFile *mf = (MemoryFile *)handle;
            if (!mf || !mf->isMemoryFile) return FILE_RESULT_ERR;
            
            uint32_t available = mf->size - mf->pos;
            uint32_t toRead = btw < available ? btw : available;
            memcpy(buff, mf->data + mf->pos, toRead);
            mf->pos += toRead;
            if (bw) *bw = toRead;
            
            return toRead == btw ? FILE_RESULT_OK : FILE_RESULT_ERR;
        }

        uint32_t fsize(FileHandle handle) {
            MemoryFile *mf = (MemoryFile *)handle;
            return mf ? mf->size : 0;
        }

        uint32_t ftell(FileHandle handle) {
            MemoryFile *mf = (MemoryFile *)handle;
            return mf ? mf->pos : 0;
        }

        FileResult fseek(FileHandle handle, uint32_t offset) {
            MemoryFile *mf = (MemoryFile *)handle;
            if (!mf || !mf->isMemoryFile) return FILE_RESULT_ERR;
            if (offset > mf->size) return FILE_RESULT_ERR;
            mf->pos = offset;
            return FILE_RESULT_OK;
        }

        FileResult fclose(FileHandle handle) {
            MemoryFile *mf = (MemoryFile *)handle;
            if (mf && mf->isMemoryFile) {
                myos::jvm::JVMBridge::FreeMemory(mf);
                return FILE_RESULT_OK;
            }
            return FILE_RESULT_ERR;
        }

        FileResult fremove(const char *fileName) { return FILE_RESULT_ERR; }
        FileResult fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
            (void)handle; (void)buff; (void)btw;
            if (bw) *bw = 0;
            return FILE_RESULT_ERR;
        }
        FileResult frename(const char *oldName, const char *newName) { return FILE_RESULT_ERR; }
        DirHandle opendir(const char *dirName) { return NULL; }
        FileResult readdir(DirHandle handle, FileInfo *fileInfo) { return FILE_RESULT_ERR; }
        FileResult closedir(DirHandle handle) { return FILE_RESULT_ERR; }
        FileResult mkdir(const char *path) { return FILE_RESULT_ERR; }
        bool isStdoutHandle(FileHandle handle) { return false; }
    }
}
