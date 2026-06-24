
#include "flint.h"
#include "flint_system_api.h"
#include "hardware_class.h"
#include "hellorpi_class.h"
#include <string.h>

struct EmbeddedClass {
    const char *name;
    const unsigned char *data;
    unsigned int size;
};

static const EmbeddedClass embeddedClasses[] = {
    {"com/rpi/Hardware.class", Hardware_class, Hardware_class_len},
    {"com/rpi/HelloRPi.class", HelloRPi_class, HelloRPi_class_len},
    {nullptr, nullptr, 0}
};

static const unsigned char *findEmbeddedClass(const char *name, unsigned int *size) {
    for (int i = 0; embeddedClasses[i].name != nullptr; i++) {
        if (strcmp(embeddedClasses[i].name, name) == 0) {
            *size = embeddedClasses[i].size;
            return embeddedClasses[i].data;
        }
    }
    return nullptr;
}

static const unsigned char *classData = nullptr;
static unsigned int classDataSize = 0;
static unsigned int classDataPos = 0;

using namespace FlintAPI::IO;

FileResult FlintAPI::IO::finfo(const char *fileName, FileInfo *fileInfo) {
    unsigned int size;
    const unsigned char *data = findEmbeddedClass(fileName, &size);
    if (data == nullptr) return FILE_RESULT_ERR;
    if (fileInfo) {
        fileInfo->size = size;
        fileInfo->directory = false;
    }
    return FILE_RESULT_OK;
}

FileHandle FlintAPI::IO::fopen(const char *fileName, FileMode mode) {
    unsigned int size;
    const unsigned char *data = findEmbeddedClass(fileName, &size);
    if (data == nullptr) return nullptr;
    classData = data;
    classDataSize = size;
    classDataPos = 0;
    return (FileHandle)1;
}

FileResult FlintAPI::IO::fread(FileHandle handle, void *buff, uint32_t btr, uint32_t *br) {
    if (handle == nullptr || classData == nullptr) return FILE_RESULT_ERR;
    uint32_t toRead = btr;
    if (classDataPos + toRead > classDataSize) {
        toRead = classDataSize - classDataPos;
    }
    memcpy(buff, classData + classDataPos, toRead);
    classDataPos += toRead;
    if (br) *br = toRead;
    return FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
    (void)handle; (void)buff; (void)btw; (void)bw;
    return FILE_RESULT_ERR;
}

uint32_t FlintAPI::IO::fsize(FileHandle handle) {
    if (handle == nullptr) return 0;
    return classDataSize;
}

uint32_t FlintAPI::IO::ftell(FileHandle handle) {
    if (handle == nullptr) return 0;
    return classDataPos;
}

FileResult FlintAPI::IO::fseek(FileHandle handle, uint32_t offset) {
    if (handle == nullptr) return FILE_RESULT_ERR;
    classDataPos = offset;
    return FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fclose(FileHandle handle) {
    (void)handle;
    classData = nullptr;
    classDataSize = 0;
    classDataPos = 0;
    return FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fremove(const char *fileName) {
    (void)fileName;
    return FILE_RESULT_ERR;
}

FileResult FlintAPI::IO::frename(const char *oldName, const char *newName) {
    (void)oldName; (void)newName;
    return FILE_RESULT_ERR;
}

DirHandle FlintAPI::IO::opendir(const char *dirName) {
    (void)dirName;
    return nullptr;
}

FileResult FlintAPI::IO::readdir(DirHandle handle, FileInfo *fileInfo) {
    (void)handle; (void)fileInfo;
    return FILE_RESULT_ERR;
}

FileResult FlintAPI::IO::closedir(DirHandle handle) {
    (void)handle;
    return FILE_RESULT_ERR;
}

FileResult FlintAPI::IO::mkdir(const char *path) {
    (void)path;
    return FILE_RESULT_ERR;
}

bool FlintAPI::IO::isStdoutHandle(FileHandle handle) {
    (void)handle;
    return false;
}
