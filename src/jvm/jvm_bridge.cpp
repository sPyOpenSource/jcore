#include <jvm/jvm_bridge.h>
#include <drivers/vga.h>
#include <memorymanagement.h>
#include <multitasking.h>
#include <gdt.h>
#include <flint_system_api.h>
#include <jvm/embedded_classes.h>
#include <vfs/vfs.h>
#include <stdio.h>
#include <string.h>

namespace myos {
    namespace jvm {
        void JVMBridge::Initialize() {
            printf("JVM Bridge initializing...\n\r");
            printf("JVM Bridge initialized.\n\r");
        }

        void* JVMBridge::AllocateMemory(myos::common::size_t size) {
            return MemoryManager::activeMemoryManager->malloc(size);
        }

        void JVMBridge::FreeMemory(void* ptr) {
            MemoryManager::activeMemoryManager->free(ptr);
        }

        void JVMBridge::Println(const char* str) {
            printf((char*)str);
            printf("\n\r");
        }
    }
}

namespace FlintAPI {
    namespace System {
        void reset(void) {}
        void *malloc(uint32_t size) { return myos::jvm::JVMBridge::AllocateMemory(size); }
        void *realloc(void *p, uint32_t size) { return myos::jvm::JVMBridge::AllocateMemory(size); }
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
            task(param);
            return (ThreadHandle)1;
        }

        ThreadHandle getCurrentThread(void) { return nullptr; }
        void terminate(ThreadHandle handle) {}
        void sleep(uint32_t ms) {}
        void yield(void) {}
    }
}

// Embedded class file fallback (for when VFS is unavailable or file not found on disk)
static const unsigned char *embeddedClassData = nullptr;
static unsigned int embeddedClassSize = 0;
static unsigned int embeddedClassPos = 0;
static const FlintAPI::IO::FileHandle EMBEDDED_HANDLE = (FlintAPI::IO::FileHandle)1;

static bool isEmbeddedHandle(FlintAPI::IO::FileHandle handle) {
    return handle == EMBEDDED_HANDLE;
}

static const unsigned char *findEmbeddedClass(const char *name, unsigned int *size) {
    for (int i = 0; embeddedClasses[i].name != nullptr; i++) {
        if (strcmp(embeddedClasses[i].name, name) == 0) {
            *size = embeddedClasses[i].size;
            return embeddedClasses[i].data;
        }
    }
    return nullptr;
}

namespace FlintAPI {
    namespace IO {
        FileResult finfo(const char *fileName, FileInfo *fileInfo) {
            if (myos::vfs::VFS::IsInitialized()) {
                myos::vfs::VFS::FileHandle handle = myos::vfs::VFS::Open(fileName, 0x01);
                if (handle) {
                    if (fileInfo) {
                        fileInfo->size = myos::vfs::VFS::Size(handle);
                        fileInfo->attribute = 0;
                        fileInfo->directory = false;
                        size_t nameLen = strlen(fileName);
                        if (nameLen > sizeof(fileInfo->name) - 1)
                            nameLen = sizeof(fileInfo->name) - 1;
                        memcpy(fileInfo->name, fileName, nameLen);
                        fileInfo->name[nameLen] = '\0';
                    }
                    printf("JVM finfo OK: "); printf(fileName); printf("\n\r");
                    myos::vfs::VFS::Close(handle);
                    return FILE_RESULT_OK;
                }
                printf("JVM finfo FAIL: "); printf(fileName); printf("\n\r");
            }
            return FILE_RESULT_NO_PATH;
        }

        FileHandle fopen(const char *fileName, FileMode mode) {
            if (myos::vfs::VFS::IsInitialized()) {
                uint8_t fatMode = 0x01;
                if (mode & FILE_MODE_WRITE) fatMode = 0x02;
                if (mode & FILE_MODE_CREATE_NEW) fatMode |= 0x10;
                printf("JVM fopen: "); printf(fileName); printf("\n\r");
                FileHandle vfsHandle = myos::vfs::VFS::Open(fileName, fatMode);
                if (vfsHandle) return vfsHandle;
                printf("JVM fopen FAILED\n\r");
            }
            return nullptr;
        }

        FileResult fread(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
            if (!handle) return FILE_RESULT_ERR;
            if (isEmbeddedHandle(handle)) {
                if (embeddedClassData == nullptr) return FILE_RESULT_ERR;
                uint32_t toRead = btw;
                if (embeddedClassPos + toRead > embeddedClassSize)
                    toRead = embeddedClassSize - embeddedClassPos;
                memcpy(buff, embeddedClassData + embeddedClassPos, toRead);
                embeddedClassPos += toRead;
                if (bw) *bw = toRead;
                return FILE_RESULT_OK;
            }
            uint32_t read = myos::vfs::VFS::Read((myos::vfs::VFS::FileHandle)handle, buff, btw);
            if (bw) *bw = read;
            return read == btw ? FILE_RESULT_OK : FILE_RESULT_ERR;
        }

        uint32_t fsize(FileHandle handle) {
            if (!handle) return 0;
            if (isEmbeddedHandle(handle)) return embeddedClassSize;
            return myos::vfs::VFS::Size((myos::vfs::VFS::FileHandle)handle);
        }

        uint32_t ftell(FileHandle handle) {
            if (!handle) return 0;
            if (isEmbeddedHandle(handle)) return embeddedClassPos;
            return myos::vfs::VFS::Tell((myos::vfs::VFS::FileHandle)handle);
        }

        FileResult fseek(FileHandle handle, uint32_t offset) {
            if (!handle) return FILE_RESULT_ERR;
            if (isEmbeddedHandle(handle)) {
                embeddedClassPos = offset;
                return FILE_RESULT_OK;
            }
            return myos::vfs::VFS::Seek((myos::vfs::VFS::FileHandle)handle, offset) ? FILE_RESULT_OK : FILE_RESULT_ERR;
        }

        FileResult fclose(FileHandle handle) {
            if (!handle) return FILE_RESULT_ERR;
#if 0
            if (isEmbeddedHandle(handle)) {
                embeddedClassData = nullptr;
                embeddedClassSize = 0;
                embeddedClassPos = 0;
                return FILE_RESULT_OK;
            }
#endif
            myos::vfs::VFS::Close((myos::vfs::VFS::FileHandle)handle);
            return FILE_RESULT_OK;
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