#include <vfs/vfs.h>
#include <drivers/ata.h>
#include <stdio.h>
#include <string.h>
#include <memorymanagement.h>

namespace myos {
    namespace vfs {
        SdVolume* VFS::s_volume = nullptr;
        SdFile* VFS::s_rootDir = nullptr;
        bool VFS::s_initialized = false;

        bool VFS::Initialize(myos::drivers::AdvancedTechnologyAttachment* device, uint8_t partition) {
            if (s_initialized) return true;

            s_volume = (SdVolume*)MemoryManager::activeMemoryManager->malloc(sizeof(SdVolume));
            if (!s_volume) {
                printf("VFS: Failed to allocate SdVolume\n");
                return false;
            }
            new (s_volume) SdVolume();

            s_rootDir = (SdFile*)MemoryManager::activeMemoryManager->malloc(sizeof(SdFile));
            if (!s_rootDir) {
                printf("VFS: Failed to allocate SdFile for root\n");
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
                return false;
            }
            new (s_rootDir) SdFile();

            if (!s_volume->init(device, partition)) {
                printf("VFS: init volume failed\n");
                s_rootDir->~SdFile();
                MemoryManager::activeMemoryManager->free(s_rootDir);
                s_rootDir = nullptr;
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
                return false;
            }

            if (!s_rootDir->openRoot(s_volume)) {
                printf("VFS: Failed to open root directory\n");
                s_rootDir->~SdFile();
                MemoryManager::activeMemoryManager->free(s_rootDir);
                s_rootDir = nullptr;
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
                return false;
            }

            s_initialized = true;
            printf("VFS: initialized\n");
            return true;
        }

        void VFS::Shutdown() {
            if (!s_initialized) return;

            if (s_rootDir) {
                s_rootDir->close();
                s_rootDir->~SdFile();
                MemoryManager::activeMemoryManager->free(s_rootDir);
                s_rootDir = nullptr;
            }
            if (s_volume) {
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
            }
            s_initialized = false;
        }

        VFS::FileHandle VFS::Open(const char* fileName, uint8_t mode) {
            if (!s_initialized || !fileName) return nullptr;

            SdFile* file = (SdFile*)MemoryManager::activeMemoryManager->malloc(sizeof(SdFile));
            if (!file) {
                printf("VFS: alloc failed\n");
                return nullptr;
            }
            new (file) SdFile();

            if (!file->open(s_rootDir, fileName, mode)) {
                printf("VFS: open fail\n");
                file->~SdFile();
                MemoryManager::activeMemoryManager->free(file);
                return nullptr;
            }

            return file;
        }

        uint32_t VFS::Read(FileHandle handle, void* buffer, uint32_t size) {
            if (!handle || !buffer || size == 0) return 0;
            SdFile* file = (SdFile*)handle;
            int16_t result = file->read(buffer, size);
            return result > 0 ? result : 0;
        }

        bool VFS::Seek(FileHandle handle, uint32_t offset) {
            if (!handle) return false;
            SdFile* file = (SdFile*)handle;
            return file->seekSet(offset) != 0;
        }

        uint32_t VFS::Tell(FileHandle handle) {
            if (!handle) return 0;
            SdFile* file = (SdFile*)handle;
            return file->curPosition();
        }

        uint32_t VFS::Size(FileHandle handle) {
            if (!handle) return 0;
            SdFile* file = (SdFile*)handle;
            return file->fileSize();
        }

        void VFS::Close(FileHandle handle) {
            if (!handle) return;
            SdFile* file = (SdFile*)handle;
            file->close();
            file->~SdFile();
            MemoryManager::activeMemoryManager->free(file);
        }

        bool VFS::Exists(const char* fileName) {
            if (!s_initialized || !fileName) return false;
            SdFile file;
            return file.open(s_rootDir, fileName, 0x01);
        }
    }
}