#include "ram_blk.h"
#include <stdint.h>
#include <stdbool.h>
#define memcpy __builtin_memcpy

#define RAM_BLK_MAX_SECTORS (64 * 1024)
static uint8_t ram_blk_storage[RAM_BLK_MAX_SECTORS * 512];
static uint32_t ram_blk_num_blocks;
static uint8_t *ram_blk_base;

static int ram_read(uint32_t lba, uint32_t count, void *buf) {
    if (lba + count > ram_blk_num_blocks) return -1;
    memcpy(buf, ram_blk_base + lba * 512, count * 512);
    return 0;
}

static int ram_write(uint32_t lba, uint32_t count, const void *buf) {
    if (lba + count > ram_blk_num_blocks) return -1;
    memcpy(ram_blk_base + lba * 512, buf, count * 512);
    return 0;
}

void ram_blk_init(struct mmc_block_dev *dev, uint32_t num_blocks) {
    ram_blk_init_at(dev, num_blocks, ram_blk_storage);
}

void ram_blk_init_at(struct mmc_block_dev *dev, uint32_t num_blocks, void *base) {
    if (num_blocks > RAM_BLK_MAX_SECTORS)
        num_blocks = RAM_BLK_MAX_SECTORS;
    ram_blk_num_blocks = num_blocks;
    ram_blk_base = (uint8_t *)base;
    dev->read_blocks = ram_read;
    dev->write_blocks = ram_write;
    dev->block_size = 512;
    dev->num_blocks = num_blocks;
    dev->is_sdhc = true;
}

void ram_blk_load(const void *data, uint32_t size) {
    if (size > sizeof(ram_blk_storage))
        size = sizeof(ram_blk_storage);
    memcpy(ram_blk_storage, data, size);
    ram_blk_num_blocks = size / 512;
    if (size % 512) ram_blk_num_blocks++;
}
