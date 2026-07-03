#include "fat32.h"

extern int dprintf(const char *fmt, ...);

#define FAT_EOC 0x0FFFFFF8

static uint32_t cluster_to_lba(struct fat32_fs *fs, uint32_t cluster) {
    return fs->first_data_sector + (cluster - 2) * fs->bpb.sectors_per_cluster;
}

static uint32_t get_fat_entry(struct fat32_fs *fs, uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sec = fs->first_fat_sector + (fat_offset / fs->bpb.bytes_per_sector);
    uint32_t ent_offset = fat_offset % fs->bpb.bytes_per_sector;

    uint8_t sector[512];
    if (fs->dev->read_blocks(fat_sec, 1, sector) != 0)
        return 0x0FFFFFFF;

    const volatile uint8_t *vp = (const volatile uint8_t *)(sector + ent_offset);
    return (vp[0] | (vp[1] << 8) | (vp[2] << 16) | (vp[3] << 24)) & 0x0FFFFFFF;
}

static uint16_t read16_le(const uint8_t *p) {
    const volatile uint8_t *vp = (const volatile uint8_t *)p;
    return (uint16_t)vp[0] | ((uint16_t)vp[1] << 8);
}

static uint32_t read32_le(const uint8_t *p) {
    const volatile uint8_t *vp = (const volatile uint8_t *)p;
    return (uint32_t)vp[0] | ((uint32_t)vp[1] << 8) |
           ((uint32_t)vp[2] << 16) | ((uint32_t)vp[3] << 24);
}

int fat32_mount(struct mmc_block_dev *dev, struct fat32_fs *fs) {
    uint8_t sector[512];

    if (dev->read_blocks(0, 1, sector) != 0)
        return -1;

    if (sector[510] != 0x55 || sector[511] != 0xAA)
        return -2;

    fs->dev = dev;
    fs->bpb.bytes_per_sector = read16_le(sector + 11);
    fs->bpb.sectors_per_cluster = sector[13];
    fs->bpb.reserved_sectors = read16_le(sector + 14);
    fs->bpb.num_fats = sector[16];

    uint16_t tot_sec_16 = read16_le(sector + 19);
    uint32_t tot_sec_32 = read32_le(sector + 32);
    fs->bpb.total_sectors = tot_sec_16 ? tot_sec_16 : tot_sec_32;

    uint16_t fat_sz_16 = read16_le(sector + 22);
    uint32_t fat_sz_32 = read32_le(sector + 36);
    fs->bpb.sectors_per_fat = fat_sz_16 ? fat_sz_16 : fat_sz_32;

    fs->bpb.root_cluster = read32_le(sector + 44);

    fs->first_fat_sector = fs->bpb.reserved_sectors;
    fs->first_data_sector = fs->bpb.reserved_sectors +
                            fs->bpb.num_fats * fs->bpb.sectors_per_fat;

    return 0;
}

static int is_dirent_free(const struct fat32_dirent *e) {
    return (uint8_t)e->name[0] == 0x00 || (uint8_t)e->name[0] == 0xE5;
}

static int is_lfn_entry(const struct fat32_dirent *e) {
    return e->attr == FAT_ATTR_LONG_NAME;
}

static int to_upper(int c) {
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

static int match_83_name(const char *filename, const char *dirent_name) {
    char name83[11];
    int i;

    /* Build 8.3 name from filename */
    int dot = -1;
    for (i = 0; filename[i]; i++) {
        if (filename[i] == '.') { dot = i; break; }
    }

    for (i = 0; i < 11; i++) name83[i] = ' ';
    if (dot < 0) {
        for (i = 0; filename[i] && i < 8; i++)
            name83[i] = to_upper(filename[i]);
    } else {
        for (i = 0; i < dot && i < 8; i++)
            name83[i] = to_upper(filename[i]);
        for (i = 0; filename[dot + 1 + i] && i < 3; i++)
            name83[8 + i] = to_upper(filename[dot + 1 + i]);
    }

    for (i = 0; i < 11; i++) {
        if (name83[i] != to_upper(dirent_name[i]))
            return 0;
    }
    return 1;
}

int fat32_open_root(struct fat32_fs *fs, const char *name, struct fat32_file *fh) {
    uint32_t cluster = fs->bpb.root_cluster;
    uint8_t buf[512];
    int entry_count = 0;

    while (cluster < FAT_EOC) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        uint32_t secs_per_clus = fs->bpb.sectors_per_cluster;

        for (uint32_t s = 0; s < secs_per_clus; s++) {
            if (fs->dev->read_blocks(lba + s, 1, buf) != 0)
                return -1;

            int entries_per_sec = fs->bpb.bytes_per_sector / FAT32_DIRENT_SIZE;
            for (int i = 0; i < entries_per_sec; i++) {
                struct fat32_dirent *e = (struct fat32_dirent *)(buf + i * FAT32_DIRENT_SIZE);

                if ((uint8_t)e->name[0] == 0x00)
                    return -3; /* end of directory */
                if (is_dirent_free(e))
                    continue;
                if (is_lfn_entry(e))
                    continue;

                if (match_83_name(name, e->name)) {
                    fh->fs = fs;
                    fh->first_cluster = ((uint32_t)e->fst_clus_hi << 16) | e->fst_clus_lo;
                    fh->size = e->file_size;
                    return 0;
                }
            }
        }

        cluster = get_fat_entry(fs, cluster);
    }

    return -4;
}

int fat32_read(struct fat32_file *fh, void *buf, uint32_t size, uint32_t offset) {
    struct fat32_fs *fs = fh->fs;
    uint32_t spc = fs->bpb.sectors_per_cluster;
    uint32_t bps = fs->bpb.bytes_per_sector;
    uint32_t cluster_size = spc * bps;

    if (offset >= fh->size)
        return 0;
    if (offset + size > fh->size)
        size = fh->size - offset;

    /* Find starting cluster */
    uint32_t skip_clusters = offset / cluster_size;
    uint32_t cluster_offset = offset % cluster_size;

    uint32_t cluster = fh->first_cluster;
    for (uint32_t i = 0; i < skip_clusters; i++) {
        cluster = get_fat_entry(fs, cluster);
        if (cluster >= FAT_EOC)
            return 0;
    }

    uint8_t *dst = (uint8_t *)buf;
    uint32_t remaining = size;

    while (remaining > 0 && cluster < FAT_EOC) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        uint32_t chunk_start = cluster_offset;
        uint32_t in_clus_remaining = cluster_size - chunk_start;
        if (in_clus_remaining > remaining)
            in_clus_remaining = remaining;

        /* Read one sector at a time within cluster */
        uint32_t bytes_read = 0;
        while (bytes_read < in_clus_remaining) {
            uint32_t sec = (chunk_start + bytes_read) / bps;
            uint32_t sec_off = (chunk_start + bytes_read) % bps;
            uint32_t to_copy = bps - sec_off;
            if (to_copy > in_clus_remaining - bytes_read)
                to_copy = in_clus_remaining - bytes_read;

            uint8_t sector[512];
            if (fs->dev->read_blocks(lba + sec, 1, sector) != 0)
                return -1;

            __builtin_memcpy(dst, sector + sec_off, to_copy);
            dst += to_copy;
            bytes_read += to_copy;
        }

        remaining -= bytes_read;
        cluster_offset = 0;
        cluster = get_fat_entry(fs, cluster);
    }

    return size - remaining;
}
