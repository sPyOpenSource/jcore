#ifndef __MYOS__VFS_H
#define __MYOS__VFS_H

#include <stdint.h>
#include <filesystem/fat.h>

namespace myos {
    namespace vfs {
        class VFS {
        private:
            static SdVolume* s_volume;
            static SdFile* s_rootDir;
            static bool s_initialized;

        public:
            static bool Initialize(myos::drivers::AdvancedTechnologyAttachment* device, uint8_t partition);
            static void Shutdown();

            static bool IsInitialized() { return s_initialized; }

            typedef SdFile* FileHandle;

            static FileHandle Open(const char* fileName, uint8_t mode = 0x01);
            static uint32_t Read(FileHandle handle, void* buffer, uint32_t size);
            static bool Seek(FileHandle handle, uint32_t offset);
            static uint32_t Tell(FileHandle handle);
            static uint32_t Size(FileHandle handle);
            static void Close(FileHandle handle);
            static bool Exists(const char* fileName);
        };
    }
}

#endif