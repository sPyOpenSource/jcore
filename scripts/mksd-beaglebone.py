#!/usr/bin/env python3
"""Generate a bootable MBR + FAT32 SD card image for BeagleBone AI."""

import struct
import sys
import os
import math

SECTOR_SIZE = 512

def make_mbr_fat32_image(total_sectors, volume_label, files, out_path):
    SPC = 1            # sectors per cluster
    NFATS = 2

    # Partition starts at sector 2048 (standard 1MB alignment)
    partition_start = 2048
    partition_sectors = total_sectors - partition_start

    # FAT32 parameters for the partition
    RSVD = 32

    # Compute FAT size iteratively
    # data area = partition_sectors - RSVD - 2*fat_sectors
    fat_sectors = 1
    for _ in range(10):
        data_clusters = (partition_sectors - RSVD - NFATS * fat_sectors) // SPC
        needed = max(1, math.ceil((data_clusters + 2) * 4 / SECTOR_SIZE))
        if needed == fat_sectors:
            break
        fat_sectors = needed

    data_clusters = (partition_sectors - RSVD - NFATS * fat_sectors) // SPC
    data_start = partition_start + RSVD + NFATS * fat_sectors
    root_cluster = 2

    img = bytearray(total_sectors * SECTOR_SIZE)

    # === MBR at LBA 0 ===
    mbr = bytearray(512)
    # Boot code (jump over partition table)
    mbr[0:3] = b'\xEB\x3C\x90'
    # Partition table entry 1 (offset 0x1BE)
    mbr[0x1BE] = 0x80  # Bootable flag
    mbr[0x1BF:0x1C2] = b'\x00\x00\x00'  # CHS start placeholder
    mbr[0x1C2] = 0x0C  # FAT32 LBA
    mbr[0x1C3:0x1C6] = b'\xFE\xFF\xFF'  # CHS end placeholder
    struct.pack_into('<I', mbr, 0x1C6, partition_start)  # LBA start
    struct.pack_into('<I', mbr, 0x1CA, partition_sectors)  # Number of sectors
    mbr[510:512] = b'\x55\xAA'
    img[0:512] = mbr

    # === Boot Sector at LBA partition_start ===
    bs = bytearray(512)
    # Jump instruction
    bs[0:3] = b'\xEB\x58\x90'
    bs[3:11] = b'MKFAT32  '
    struct.pack_into('<H', bs, 11, SECTOR_SIZE)  # BPB_BytsPerSec
    bs[13] = SPC                                 # BPB_SecPerClus
    struct.pack_into('<H', bs, 14, RSVD)       # BPB_RsvdSecCnt
    bs[16] = NFATS                                # BPB_NumFATs
    struct.pack_into('<H', bs, 17, 0)           # BPB_RootEntCnt (0 for FAT32)
    struct.pack_into('<H', bs, 19, 0)           # BPB_TotSec16
    bs[21] = 0xF8                                 # BPB_Media
    struct.pack_into('<H', bs, 22, 0)           # BPB_FATSz16
    struct.pack_into('<H', bs, 24, 0)           # BPB_SecPerTrk
    struct.pack_into('<H', bs, 26, 0)           # BPB_NumHeads
    struct.pack_into('<I', bs, 28, partition_start)  # BPB_HiddSec
    struct.pack_into('<I', bs, 32, partition_sectors) # BPB_TotSec32

    # FAT32 specific
    struct.pack_into('<I', bs, 36, fat_sectors)   # BPB_FATSz32
    bs[40] = 0                                     # BPB_ExtFlags
    bs[41] = 0
    struct.pack_into('<H', bs, 42, 0)             # BPB_FSVer
    struct.pack_into('<I', bs, 44, root_cluster)  # BPB_RootClus
    struct.pack_into('<H', bs, 48, 1)             # BPB_FSInfo
    struct.pack_into('<H', bs, 50, 6)             # BPB_BkBootSec
    bs[53] = 0x80                                 # BPB_DrvNum
    bs[54] = 0                                     # BPB_Reserved1
    bs[55] = 0x29                                 # BPB_BootSig
    struct.pack_into('<I', bs, 56, 0x12345678)    # BS_VolID
    vol_label_padded = volume_label.ljust(11, ' ')[:11].upper().encode('ascii')
    bs[59:71] = vol_label_padded
    bs[71:82] = b'FAT32   '
    bs[82:510] = b'\x00' * (510 - 82)
    bs[510:512] = b'\x55\xAA'

    img[partition_start * SECTOR_SIZE:(partition_start + 1) * SECTOR_SIZE] = bs

    # === FSInfo sector (LBA partition_start + 1) ===
    fsi = bytearray(512)
    struct.pack_into('<I', fsi, 0, 0x41615252)       # FSI_LeadSig
    struct.pack_into('<I', fsi, 484, 0x61417272)     # FSI_StrucSig
    struct.pack_into('<I', fsi, 488, data_clusters)  # FSI_Free_Count
    struct.pack_into('<I', fsi, 492, 3)              # FSI_Nxt_Free
    struct.pack_into('<I', fsi, 508, 0xAA550000)     # FSI_TrailSig
    img[(partition_start + 1) * SECTOR_SIZE:(partition_start + 2) * SECTOR_SIZE] = fsi

    # === Backup boot sector (LBA partition_start + 6) ===
    img[(partition_start + 6) * SECTOR_SIZE:(partition_start + 7) * SECTOR_SIZE] = bs.copy()

    # === FAT tables ===
    fat_size_bytes = fat_sectors * SECTOR_SIZE
    fat_data = bytearray(fat_size_bytes)
    struct.pack_into('<I', fat_data, 0, 0x0FFFFFF8)   # cluster 0
    struct.pack_into('<I', fat_data, 4, 0x0FFFFFFF)   # cluster 1
    struct.pack_into('<I', fat_data, root_cluster * 4, 0x0FFFFFFF)  # root dir

    dir_entries = bytearray()
    next_free_cluster = root_cluster + 1

    for name, content in files:
        name_upper = name.upper()
        basename, ext = os.path.splitext(name_upper)
        ext = ext.lstrip('.')
        fname_83 = basename.ljust(8, ' ')[:8] + ext.ljust(3, ' ')[:3]

        start_cluster = next_free_cluster

        entry = bytearray(32)
        entry[0:11] = fname_83.encode('ascii')
        entry[11] = 0x20  # archive attribute
        entry[13] = 0
        struct.pack_into('<H', entry, 14, (12 << 11) | (0 << 5) | 0)
        struct.pack_into('<H', entry, 16, ((2024 - 1980) << 9) | (1 << 5) | 1)
        struct.pack_into('<H', entry, 18, ((2024 - 1980) << 9) | (1 << 5) | 1)
        struct.pack_into('<H', entry, 20, (start_cluster >> 16) & 0xFFFF)
        struct.pack_into('<H', entry, 22, (12 << 11) | (0 << 5) | 0)
        struct.pack_into('<H', entry, 24, ((2024 - 1980) << 9) | (1 << 5) | 1)
        struct.pack_into('<H', entry, 26, start_cluster & 0xFFFF)
        struct.pack_into('<I', entry, 28, len(content))

        dir_entries.extend(entry)

        # Write file data to clusters and mark FAT chain
        pos = 0
        cluster = start_cluster
        while pos < len(content):
            chunk = content[pos:pos + SECTOR_SIZE * SPC]
            cluster_offset = data_start + (cluster - 2) * SPC
            img[cluster_offset * SECTOR_SIZE:cluster_offset * SECTOR_SIZE + len(chunk)] = chunk
            pos += len(chunk)
            next_cluster = cluster + 1 if pos < len(content) else 0x0FFFFFFF
            struct.pack_into('<I', fat_data, cluster * 4, next_cluster)
            cluster += 1

        next_free_cluster = cluster

    # Root directory (cluster 2)
    root_offset = (data_start + (root_cluster - 2) * SPC) * SECTOR_SIZE
    dir_data = bytearray(SPC * SECTOR_SIZE)
    dir_data[:len(dir_entries)] = dir_entries
    img[root_offset:root_offset + SPC * SECTOR_SIZE] = dir_data

    # Write FATs
    fat1_offset = (partition_start + RSVD) * SECTOR_SIZE
    fat2_offset = (partition_start + RSVD + fat_sectors) * SECTOR_SIZE
    img[fat1_offset:fat1_offset + fat_size_bytes] = fat_data
    img[fat2_offset:fat2_offset + fat_size_bytes] = fat_data

    with open(out_path, 'wb') as f:
        f.write(img)

    print(f"MBR + FAT32 image: {out_path}")
    size_mb = total_sectors * SECTOR_SIZE / 1024 / 1024
    print(f"  Total sectors: {total_sectors} ({size_mb:.1f} MB)")
    print(f"  Partition start: LBA {partition_start}")
    print(f"  Data start: LBA {data_start}")
    print(f"  Data clusters: {data_clusters}")
    print("  Files:")
    for name, content in files:
        print(f"    {name} ({len(content)} bytes)")


if __name__ == '__main__':
    import json
    with open(sys.argv[1]) as f:
        config = json.load(f)

    files = []
    for f in config['files']:
        if 'data_hex' in f:
            files.append((f['name'], bytes.fromhex(f['data_hex'])))
        elif 'data_file' in f:
            with open(f['data_file'], 'rb') as bf:
                files.append((f['name'], bf.read()))
        else:
            files.append((f['name'], b''))

    make_mbr_fat32_image(config['total_sectors'], config['volume_label'],
                         files, config['out_path'])
