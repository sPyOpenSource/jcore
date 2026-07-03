# VFS and FAT32 on MMC2 Implementation Plan

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Implement a minimal VFS and FAT32 filesystem on the AM572x MMC2 controller to load model weights from an SD card.

**Architecture:** `VFS Layer` $\rightarrow$ `FAT32 Driver` $\rightarrow$ `MMC2 Block Driver` $\rightarrow$ `Hardware`.
**Tech Stack:** C, AM572x Hardware Registers, FAT32 Specification.

---

## Phase 1: Infrastructure & Hardware Enablement

### Task 1: Implement PRCM for MMC2
**Objective:** Enable power and clock for the MMC2 controller.
**Files:**
- Create: `src/drivers/prcm.c`, `src/drivers/prcm.h`
- Modify: `src/main.c`

**Step 1: Implement `prcm_enable_mmc2()`**
Write code to enable `CM_L4PER_MMC2_CLKCTRL` and wait for `IDLEST == FUNC`.

**Step 2: Call in `main()`**
Add `prcm_enable_mmc2()` to `main()` before any MMC2 access.

**Step 3: Commit**
`git commit -m "feat: add PRCM support for MMC2"`

### Task 2: Implement Pinmux for MMC2
**Objective:** Configure pins for MMC2 (CLK, CMD, DAT0-3).
**Files:**
- Create: `src/drivers/pinmux.c`, `src/drivers/pinmux.h`
- Modify: `src/main.c`

**Step 1: Implement `pinmux_init_mmc2()`**
Set the correct mux modes for MMC2 pins as per AM572x TRM.

**Step 2: Call in `main()`**
Add `pinmux_init_mmc2()` after PRCM enable.

**Step 3: Commit**
`git commit -m "feat: add pinmux config for MMC2"`

---

## Phase 2: MMC2 Block Driver (PIO)

### Task 3: Basic MMC2 Register Interface
**Objective:** Create low-level wrappers for MMC2 registers.
**Files:**
- Create: `src/drivers/mmc2.c`, `src/drivers/mmc2.h`

**Step 1: Define register offsets and helper macros**
Implement `mmc2_write()` and `mmc2_read()`.

**Step 2: Commit**
`git commit -m "feat: add MMC2 register interface"`

### Task 4: MMC2 Command & Response Logic
**Objective:** Implement sending commands and waiting for responses.
**Files:**
- Modify: `src/drivers/mmc2.c`

**Step 1: Implement `mmc2_send_cmd(uint32_t cmd, uint32_t arg, uint32_t resp_type)`**
Handle command phase and response timeout.

**Step 2: Commit**
`git commit -m "feat: implement MMC2 command/response logic"`

### Task 5: SD Card Identification Sequence
**Objective:** Perform the full SD initialization sequence.
**Files:**
- Modify: `src/drivers/mmc2.c`

**Step 1: Implement `mmc2_init_sd()`**
Sequence: CMD0 $\rightarrow$ CMD8 $\rightarrow$ ACMD41 $\rightarrow$ CMD2 $\rightarrow$ CMD3 $\rightarrow$ CMD7 $\rightarrow$ ACMD6 (4-bit).

**Step 2: Commit**
`git commit -m "feat: implement SD card initialization sequence"`

### Task 6: PIO Block Read/Write
**Objective:** Implement basic sector reading/writing via PIO.
**Files:**
- Modify: `src/drivers/mmc2.c`

**Step 1: Implement `mmc2_read_blocks(uint32_t lba, uint32_t count, void *buf)`**
CMD17 $\rightarrow$ read from data port FIFO.

**Step 2: Implement `mmc2_write_blocks(...)`**
CMD24 $\rightarrow$ write to data port FIFO.

**Step 3: Commit**
`git commit -m "feat: implement PIO block read/write for MMC2"`

---

## Phase 3: FAT32 Driver

### Task 7: Mock Block Driver for QEMU Testing
**Objective:** Create a RAM-backed block driver to test FAT32 logic in QEMU.
**Files:**
- Create: `src/drivers/ram_blk.c`, `src/drivers/ram_blk.h`

**Step 1: Implement `ram_blk_read/write`**
Use a static buffer to simulate a disk image.

**Step 2: Commit**
`git commit -m "test: add mock RAM block driver for QEMU"`

### Task 8: FAT32 BPB and FSInfo Parsing
**Objective:** Parse the boot sector to find FAT and data areas.
**Files:**
- Create: `src/fs/fat32.c`, `src/fs/fat32.h`

**Step 1: Implement `fat32_mount(struct mmc_block_dev *blk)`**
Parse BPB, verify FAT32 signature, calculate `fat_start_lba` and `data_start_lba`.

**Step 2: Commit**
`git commit -m "feat: implement FAT32 BPB parsing"`

### Task 9: FAT32 Directory Entry Parsing
**Objective:** Walk directory entries to find files.
**Files:**
- Modify: `src/fs/fat32.c`

**Step 1: Implement `fat32_find_file(const char *path)`**
Search root directory and subdirectories for a matching name.

**Step 2: Commit**
`git commit -m "feat: implement FAT32 directory search"`

### Task 10: FAT32 Cluster Chain Traversal
**Objective:** Read file data by following the FAT chain.
**Files:**
- Modify: `src/fs/fat32.c`

**Step 1: Implement `fat32_read_file(struct fat32_file *file, void *buf, size_t len)`**
Follow FAT entries to read sequential sectors.

**Step 2: Commit**
`git commit -m "feat: implement FAT32 cluster chain read"`

---

## Phase 4: VFS Layer

### Task 11: VFS Core Infrastructure
**Objective:** Implement the mount table and FD dispatch.
**Files:**
- Create: `src/vfs.c`, `src/vfs.h`

**Step 1: Implement `vfs_init()` and `vfs_mount()`**.
Step 2: Implement `vfs_open`, `vfs_read`, `vfs_close` using `vfs_ops` dispatch.

**Step 3: Commit**
`git commit -m "feat: implement VFS core layer"`

### Task 12: FAT32 VFS Integration
**Objective:** Connect FAT32 driver to the VFS layer.
**Files:**
- Modify: `src/fs/fat32.c`, `src/vfs.c`

**Step 1: Implement `fat32_vfs_ops`**
Map `vfs_ops` functions to `fat32_open/read/close`.

**Step 2: Commit**
`git commit -m "feat: integrate FAT32 into VFS"`

---

## Phase 5: Integration & Verification

### Task 13: Loading Model from SD Card
**Objective:** Replace embedded model weights with VFS-based loading.
**Files:**
- Modify: `src/main.c`, `src/llm.c`

**Step 1: Add VFS init and mount to `main()`**
Call `vfs_init()` and `fat32_mount("/sd", &mmc2_blk)`.

**Step 2: Modify `llm()` to use VFS**
Change `build_transformer(&t, _binary_model_bin_start)` to a loop of `vfs_open("/sd/model.bin", O_RDONLY, &f)` $\rightarrow$ `vfs_read()` $\rightarrow$ buffer.

**Step 3: Commit**
`git commit -m "feat: load llama2 model from SD card via VFS"`

### Task 14: Final Verification on BBAI
**Objective:** End-to-end test on hardware.
**Steps:**
1. Flash kernel to BBAI.
2. Insert SD card with `model.bin` formatted as FAT32.
3. Verify UART output shows "loading model... OK" and generates tokens.

**Step 5: Commit**
`git commit -m "test: verify VFS+FAT32 on BBAI hardware"`
