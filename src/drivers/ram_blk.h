#ifndef RAM_BLK_H
#define RAM_BLK_H

#include <stdint.h>
#include <stdbool.h>
#include "mmc2.h"

/* Initialize the RAM-backed block device with a given number of 512-byte sectors.
   Uses an internal static buffer. */
void ram_blk_init(struct mmc_block_dev *dev, uint32_t num_blocks);

/* Initialize with an external pre-loaded buffer at the given address */
void ram_blk_init_at(struct mmc_block_dev *dev, uint32_t num_blocks, void *base);

/* Load a buffer as the backing storage */
void ram_blk_load(const void *data, uint32_t size);

#endif /* RAM_BLK_H */
