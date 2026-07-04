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
                return false;
            }
            new (s_volume) SdVolume();

            s_rootDir = (SdFile*)MemoryManager::activeMemoryManager->malloc(sizeof(SdFile));
            if (!s_rootDir) {
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
                return false;
            }
            new (s_rootDir) SdFile();

            if (!s_volume->init(device, partition)) {
                s_rootDir->~SdFile();
                MemoryManager::activeMemoryManager->free(s_rootDir);
                s_rootDir = nullptr;
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
                return false;
            }

            if (!s_rootDir->openRoot(s_volume)) {
                s_rootDir->~SdFile();
                MemoryManager::activeMemoryManager->free(s_rootDir);
                s_rootDir = nullptr;
                s_volume->~SdVolume();
                MemoryManager::activeMemoryManager->free(s_volume);
                s_volume = nullptr;
                return false;
            }

            s_initialized = true;
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

        static const char* nextComponent(const char* path, char* buf, size_t bufSize) {
            while (*path == '/') path++;
            if (!*path) return nullptr;
            size_t i = 0;
            while (*path && *path != '/' && i + 1 < bufSize) {
                buf[i++] = *path++;
            }
            buf[i] = '\0';
            return path;
        }

        VFS::FileHandle VFS::Open(const char* fileName, uint8_t mode) {
            if (!s_initialized || !fileName) return nullptr;

            char comp[64];
            const char* remaining = nextComponent(fileName, comp, sizeof(comp));
            if (!remaining && comp[0] == '\0') return nullptr;

            SdFile* dir = (SdFile*)MemoryManager::activeMemoryManager->malloc(sizeof(SdFile));
            if (!dir) return nullptr;
            new (dir) SdFile();
            if (!dir->openRoot(s_volume)) {
                dir->~SdFile();
                MemoryManager::activeMemoryManager->free(dir);
                return nullptr;
            }

            while (true) {
                uint8_t openMode = remaining ? 0x01 : mode;

                SdFile* child = (SdFile*)MemoryManager::activeMemoryManager->malloc(sizeof(SdFile));
                if (!child) {
                    dir->close(); dir->~SdFile(); MemoryManager::activeMemoryManager->free(dir);
                    return nullptr;
                }
                new (child) SdFile();

                if (!child->open(dir, comp, openMode)) {
                    child->~SdFile(); MemoryManager::activeMemoryManager->free(child);
                    dir->close(); dir->~SdFile(); MemoryManager::activeMemoryManager->free(dir);
                    return nullptr;
                }

                dir->close(); dir->~SdFile(); MemoryManager::activeMemoryManager->free(dir);
                dir = child;

                if (!remaining || !*remaining) break;
                remaining = nextComponent(remaining, comp, sizeof(comp));
            }

            return dir;
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
            SdFile* handle = Open(fileName, 0x01);
            if (handle) {
                Close(handle);
                return true;
            }
            return false;
        }
    }
}