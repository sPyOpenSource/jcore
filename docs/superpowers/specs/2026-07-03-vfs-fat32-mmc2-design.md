# VFS and FAT32 on AM572x MMC2 Design

**Date:** 2026-07-03
**Status:** Proposed
**Owner:** opencode

## 1. Introduction

This document defines the design for adding a Virtual File System (VFS) layer and a FAT32 filesystem implementation targeting the AM572x MMC2 controller on the BeagleBone AI. The primary goal is to move the Llama2 model weights from being embedded in the ELF binary to being loaded from an SD card at runtime.

### 1.1 Success Criteria
- Ability to mount an SD card using the MMC2 controller.
- Implementation of a minimal VFS (`open`, `read`, `close`).
- Implementation of FAT32 parsing (BPB, FAT chains, Directory Entries).
- Successfully loading `model.bin` from the SD card and passing it to the `build_transformer` function.
- Support for 4-bit wide bus transfers via PIO.

## 2. Architecture

The system follows a layered approach:

`Application` $\rightarrow$ `VFS Layer` $\rightarrow$ `FAT32 Driver` $\rightarrow$ `MMC2 Block Driver` $\rightarrow$ `Hardware`

### 2.1 VFS Layer
The VFS provides a uniform interface for accessing different storage backends.

- **Mount Table**: Maps path prefixes (e.g., `/sd`) to specific filesystem implementations.
- **File Descriptor (FD) Table**: Tracks open files and their current positions.
- **API**:
    - `int vfs_open(const char *path, int flags)`
    - `int vfs_read(int fd, void *buf, size_t count, size_t *bytes_read)`
    - `int vfs_close(int fd)`

### 2.2 FAT32 Driver
The FAT32 driver translates VFS file operations into block requests.

- **BPB (Bios Parameter Block)**: Parses the boot sector to determine sector size, cluster size, and FAT locations.
- **Cluster Chain**: Traverses the File Allocation Table (FAT) to resolve fragmented files.
- **Directory Entries**: Supports Short File Names (SFN). Long File Name (LFN) support is optional but recommended for `model.bin`.
- **State**: Maintains a `fat32_file` structure tracking the current cluster and offset.

### 2.3 MMC2 Block Driver (AM572x)
The hardware driver manages the physical SD card via the AM572x MMC2 controller.

- **Interface**: `int mmc_read_blocks(uint32_t lba, uint32_t count, void *buf)`
- **Initialization Sequence**:
    1. Power/Clock enable via PRCM.
    2. Pinmux configuration.
    3. SD identification phase (CMD0 $\rightarrow$ CMD8 $\rightarrow$ ACMD41).
    4. Transfer phase (CMD2 $\rightarrow$ CMD3 $\rightarrow$ CMD7).
    5. Bus width switch to 4-bit (ACMD6).
- **PIO Transfers**: CPU-driven reads/writes via the data port FIFO.

## 3. Component Details

### 3.1 Data Structures

```c
typedef struct vfs_ops {
    int (*open)(const char *path, int flags, void **priv);
    int (*read)(void *priv, void *buf, size_t count, size_t *read);
    int (*close)(void *priv);
} vfs_ops_t;

typedef struct vfs_mount {
    const char *prefix;
    const vfs_ops_t *ops;
    void *fs_data;
} vfs_mount_t;

typedef struct fat32_fs {
    uint32_t sector_size;
    uint32_t sectors_per_cluster;
    uint32_t fat_start_lba;
    uint32_t data_start_lba;
    uint32_t root_cluster;
    struct mmc_block_dev *block_dev;
} fat32_fs_t;
```

### 3.2 Memory Map & Hardware
- **MMC2 Base Address**: Per AM572x TRM.
- **Clock Domain**: `CM_L4PER_MMC2_CLKCTRL`.
- **Sectors**: 512 bytes per sector.

## 4. Implementation Plan Highlights

1. **Hardware Enablement**: Implement PRCM and Pinmux for MMC2.
2. **MMC Protocol**: Implement CMD/Response loop and 4-bit mode switch.
3. **FAT32 Core**: Implement BPB parsing and cluster traversal.
4. **VFS Integration**: Implement the mount table and dispatch logic.
5. **Verification**: 
    - Use a mock block driver in QEMU to verify FAT32 logic using a disk image.
    - Deploy to BBAI to verify physical SD card access.

## 5. Error Handling

- **MMC Timeouts**: All CMD/DAT waits have a fixed timeout. Return `-EIO`.
- **FAT Corruptions**: Invalid cluster IDs or BPB values result in `-EILSEQ`.
- **VFS**: Return standard `errno` values (e.g., `ENOENT` for file not found).
