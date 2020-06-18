/*
 * mkbootimg/util.c
 *
 * Copyright (C) 2017 - 2020 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Utility functions
 *
 */
#include "main.h"

time_t t;
struct tm *ts;
guid_t diskguid;
char *json = NULL, *config = NULL, *kernelname = NULL, *initrd_dir[NUMARCH] = {0}, *initrd_file[NUMARCH] = {0};
char initrd_arch[NUMARCH] = {0};
int fs_len, initrd_size[NUMARCH] = {0}, initrd_gzip = 1, boot_size = 0, boot_fat = 16, disk_size = 0;
int iso9660 = 0, skipbytes = 0;
unsigned char *fs_base = NULL, *initrd_buf[NUMARCH] = {0};
unsigned long int tsize = 0, es = 0, esiz = 0, disk_align = 0;
initrd_open rd_open = NULL;
initrd_add rd_add = NULL;
initrd_close rd_close = NULL;

/**
 * Read a file entirely into memory. Don't use it with partition image files
 */
long int read_size;
unsigned char* readfileall(char *file)
{
    unsigned char *data=NULL;
    FILE *f;
    read_size=0;
    if(!file || !*file) return NULL;
    f=fopen(file,"r");
    if(f){
        fseek(f,0L,SEEK_END);
        read_size=ftell(f);
        fseek(f,0L,SEEK_SET);
        data=(unsigned char*)malloc(read_size+1);
        if(!data) { fprintf(stderr,"mkbootimg: unable to allocate %ld memory\r\n",read_size+1); exit(1); }
        memset(data,0,read_size+1);
        fread(data,read_size,1,f);
        data[read_size] = 0;
        fclose(f);
    }
    return data;
}

/**
 * Convert hex string into integer
 */
unsigned int gethex(char *ptr, int len)
{
    unsigned int ret = 0;
    for(;len--;ptr++) {
        if(*ptr>='0' && *ptr<='9') {          ret <<= 4; ret += (unsigned int)(*ptr-'0'); }
        else if(*ptr >= 'a' && *ptr <= 'f') { ret <<= 4; ret += (unsigned int)(*ptr-'a'+10); }
        else if(*ptr >= 'A' && *ptr <= 'F') { ret <<= 4; ret += (unsigned int)(*ptr-'A'+10); }
        else break;
    }
    return ret;
}

/**
 * Parse a GUID in string into binary representation
 */
void getguid(char *ptr, guid_t *guid)
{
    int i;
    memset(guid, 0, sizeof(guid_t));
    if(!ptr || !*ptr || ptr[8] != '-' || ptr[13] != '-' || ptr[18] != '-') return;
    guid->Data1 = gethex(ptr, 8); ptr += 9;
    guid->Data2 = gethex(ptr, 4); ptr += 5;
    guid->Data3 = gethex(ptr, 4); ptr += 5;
    guid->Data4[0] = gethex(ptr, 2); ptr += 2;
    guid->Data4[1] = gethex(ptr, 2); ptr += 2; if(*ptr == '-') ptr++;
    for(i = 2; i < 8; i++, ptr += 2) guid->Data4[i] = gethex(ptr, 2);
}

/**
 * Recursively parse a directory
 */
void parsedir(char *directory, int parent)
{
    DIR *dir;
    struct dirent *ent;
    char full[1024];
    unsigned char *tmp;
    struct stat st;

    if ((dir = opendir(directory)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if(ent->d_name[0] == '.') continue;
            sprintf(full,"%s/%s",directory,ent->d_name);
            if(stat(full, &st)) continue;
            if(S_ISDIR(st.st_mode)) {
                (*rd_add)(&st, full + skipbytes, NULL, 0);
                parsedir(full, parent+1);
            } else {
                tmp = NULL; read_size = 0;
                if(S_ISREG(st.st_mode)) tmp = readfileall(full);
                (*rd_add)(&st, full + skipbytes, tmp, read_size);
                if(tmp) free(tmp);
            }
        }
        closedir(dir);
    }
}

/**
 * Compress the initrd image
 */
void initrdcompress()
{
    unsigned char *initrdgz;
    unsigned long int initrdgz_len = 0;
    uint32_t crc;
    if(!initrd_gzip || !fs_len || !fs_base) return;
    initrdgz_len = compressBound(fs_len);
    initrdgz = malloc(initrdgz_len);
    if(!initrdgz) { fprintf(stderr,"mkbootimg: unable to allocate memory\r\n"); exit(1); }
    compress2(initrdgz, &initrdgz_len, fs_base, fs_len, 9);
    if(initrdgz_len) {
        crc = crc32(0,fs_base, fs_len);
        fs_base = realloc(fs_base, initrdgz_len + 18);
        if(!fs_base) { fprintf(stderr,"mkbootimg: unable to allocate memory\r\n"); exit(1); }
        memset(fs_base, 0, 10);
        fs_base[0] = 0x1f; fs_base[1] = 0x8b; fs_base[2] = 0x8; fs_base[9] = 3;
        memcpy(fs_base + 4, &t, 4);
        memcpy(fs_base + 10, initrdgz + 2, initrdgz_len - 2);
        memcpy(fs_base + 10 + initrdgz_len - 2, &crc, 4);
        memcpy(fs_base + 14 + initrdgz_len - 2, &fs_len, 4);
        fs_len = initrdgz_len - 2 + 18;
    }
}
