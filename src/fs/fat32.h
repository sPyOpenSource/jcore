#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include "drivers/mmc2.h"

#define FAT32_DIRENT_SIZE 32
#define FAT32_MAX_NAME 12

/* FAT32 BPB from boot sector (LBA 0).
 * All fields stored at naturally-aligned offsets to avoid
 * unaligned access faults on ARM (strh/str to odd addresses). */
struct fat32_bpb {
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint8_t  _pad0;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint8_t  _pad1;
    uint8_t  _pad2;
    uint8_t  _pad3;
    uint32_t total_sectors;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
};

/* FAT32 filesystem state */
struct fat32_fs {
    struct mmc_block_dev *dev;
    struct fat32_bpb bpb;
    uint32_t first_fat_sector;
    uint32_t first_data_sector;
};

/* FAT32 directory entry (8.3 format, 32 bytes) */
struct fat32_dirent {
    char     name[11];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;
} __attribute__((packed));

/* FAT32 attribute bits */
#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_ATTR_LONG_NAME  0x0F

/* FAT32 file handle for cluster chain traversal */
struct fat32_file {
    struct fat32_fs *fs;
    uint32_t first_cluster;
    uint32_t size;
};

/* API */
int fat32_mount(struct mmc_block_dev *dev, struct fat32_fs *fs);
int fat32_open_root(struct fat32_fs *fs, const char *name, struct fat32_file *fh);
int fat32_read(struct fat32_file *fh, void *buf, uint32_t size, uint32_t offset);

#endif /* FAT32_H */
