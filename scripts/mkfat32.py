#!/usr/bin/env python3
"""Generate a minimal FAT32 image with specified files."""

import struct
import sys
import os
import math

SECTOR_SIZE = 512

def make_fat32_image(total_sectors, volume_label, files, out_path):
    SPB = SECTOR_SIZE  # bytes per sector
    SPC = 1            # sectors per cluster
    RSVD = 32          # reserved sectors
    NFATS = 2
    root_cluster = 2   # first cluster is cluster 2

    # Compute FAT size iteratively
    data_clusters_est = (total_sectors - RSVD) // SPC
    fat_sectors = max(1, math.ceil((data_clusters_est * 4) / SPB))

    # Iterate to converge
    for _ in range(10):
        data_start = RSVD + NFATS * fat_sectors
        data_clusters = (total_sectors - data_start) // SPC
        needed = max(1, math.ceil((data_clusters + 2) * 4 / SPB))
        if needed == fat_sectors:
            break
        fat_sectors = needed

    data_start = RSVD + NFATS * fat_sectors
    data_clusters = (total_sectors - data_start) // SPC

    total_sectors_16 = 0
    total_sectors_32 = total_sectors
    if total_sectors < 0x10000:
        total_sectors_16 = total_sectors
        total_sectors_32 = 0

    # Root directory entries count (0 for FAT32)
    root_entries = 0

    # Count root dir entries for our files
    # Each file needs one directory entry
    # We'll pack them right after the root cluster

    img = bytearray(total_sectors * SPB)

    # ---- Boot Sector (LBA 0) ----
    bs = bytearray(512)
    # jmp instruction
    bs[0:3] = b'\xEB\x58\x90'
    # OEM name
    bs[3:11] = b'MKFAT32  '
    # BPB
    struct.pack_into('<H', bs, 11, SPB)         # BPB_BytsPerSec
    bs[13] = SPC                                 # BPB_SecPerClus
    struct.pack_into('<H', bs, 14, RSVD)         # BPB_RsvdSecCnt
    bs[16] = NFATS                                # BPB_NumFATs
    struct.pack_into('<H', bs, 17, root_entries)  # BPB_RootEntCnt
    struct.pack_into('<H', bs, 19, total_sectors_16)  # BPB_TotSec16
    bs[21] = 0xF8                                 # BPB_Media (hard disk)
    struct.pack_into('<H', bs, 22, 0)             # BPB_FATSz16 (0 for FAT32)
    struct.pack_into('<H', bs, 24, 0)             # BPB_SecPerTrk
    struct.pack_into('<H', bs, 26, 0)             # BPB_NumHeads
    struct.pack_into('<I', bs, 28, 0)             # BPB_HiddSec
    struct.pack_into('<I', bs, 32, total_sectors_32)  # BPB_TotSec32

    # FAT32 specific
    struct.pack_into('<I', bs, 36, fat_sectors)   # BPB_FATSz32
    bs[40] = 0                                    # BPB_ExtFlags
    bs[41] = 0
    struct.pack_into('<H', bs, 42, 0)             # BPB_FSVer
    struct.pack_into('<I', bs, 44, root_cluster)  # BPB_RootClus
    struct.pack_into('<H', bs, 48, 1)             # BPB_FSInfo
    struct.pack_into('<H', bs, 50, 6)             # BPB_BkBootSec
    # Reserved[12]
    bs[53] = 0x80                                 # BPB_DrvNum
    bs[54] = 0                                    # BPB_Reserved1
    bs[55] = 0x29                                 # BPB_BootSig
    struct.pack_into('<I', bs, 56, 0x12345678)    # BS_VolID
    # Volume label
    vol_label_padded = volume_label.ljust(11, ' ')[:11].upper().encode('ascii')
    bs[59:71] = vol_label_padded
    # FS type
    bs[71:82] = b'FAT32   '
    # Bootstrap
    bs[82:510] = b'\x00' * (510 - 82)
    bs[510:512] = b'\x55\xAA'

    img[0:512] = bs

    # ---- FSInfo sector (LBA 1) ----
    fsi = bytearray(512)
    struct.pack_into('<I', fsi, 0, 0x41615252)       # FSI_LeadSig
    struct.pack_into('<I', fsi, 484, 0x61417272)     # FSI_StrucSig
    struct.pack_into('<I', fsi, 488, data_clusters)  # FSI_Free_Count (approximate)
    struct.pack_into('<I', fsi, 492, 3)              # FSI_Nxt_Free (first usable cluster)
    struct.pack_into('<I', fsi, 508, 0xAA550000)     # FSI_TrailSig
    img[512:1024] = fsi

    # ---- Backup boot sector (LBA 6) ----
    img[6*512:7*512] = bs.copy()

    # ---- FAT #1 (starts at LBA RSVD = 32) ----
    fat_entries = data_clusters + 2
    fat1_offset = RSVD * SPB
    fat_size_bytes = fat_sectors * SPB

    # Cluster 0: 0x0FFFFFF8 (media descriptor)
    # Cluster 1: 0x0FFFFFFF (end of chain / EOC)
    # Cluster 2+: data clusters

    fat_data = bytearray(fat_size_bytes)
    struct.pack_into('<I', fat_data, 0, 0x0FFFFFF8)   # cluster 0
    struct.pack_into('<I', fat_data, 4, 0x0FFFFFFF)   # cluster 1

    # Root dir takes cluster 2; files start at cluster 3
    next_free_cluster = root_cluster + 1

    # Mark root cluster as EOC in FAT
    struct.pack_into('<I', fat_data, root_cluster * 4, 0x0FFFFFFF)

    # Build directory entries
    dir_entries = bytearray()
    file_clusters = {}

    for name, content in files:
        # Convert filename to 8.3 format
        name_upper = name.upper()
        basename, ext = os.path.splitext(name_upper)
        ext = ext.lstrip('.')
        fname_83 = basename.ljust(8, ' ')[:8] + ext.ljust(3, ' ')[:3]

        # Allocate clusters for this file
        file_size = len(content)
        start_cluster = next_free_cluster

        entry = bytearray(32)
        entry[0:11] = fname_83.encode('ascii')
        entry[11] = 0x20  # archive attribute
        entry[13] = 0     # create time tenths
        struct.pack_into('<H', entry, 14, (12 << 11) | (0 << 5) | 0)  # create time: 12:00:00
        struct.pack_into('<H', entry, 16, ((2024 - 1980) << 9) | (1 << 5) | 1)  # create date: 2024-01-01
        struct.pack_into('<H', entry, 18, ((2024 - 1980) << 9) | (1 << 5) | 1)  # last access: 2024-01-01
        struct.pack_into('<H', entry, 20, (start_cluster >> 16) & 0xFFFF)  # DIR_FstClusHI (FAT32)
        struct.pack_into('<H', entry, 22, (12 << 11) | (0 << 5) | 0)      # last modified time: 12:00:00
        struct.pack_into('<H', entry, 24, ((2024 - 1980) << 9) | (1 << 5) | 1)  # last modified date
        struct.pack_into('<H', entry, 26, start_cluster & 0xFFFF)  # DIR_FstClusLO
        struct.pack_into('<I', entry, 28, file_size)                # DIR_FileSize

        dir_entries.extend(entry)

        # Write file data to clusters and mark FAT chain
        cluster = start_cluster
        pos = 0
        while pos < file_size:
            chunk = content[pos:pos + SPB * SPC]
            cluster_offset = data_start + (cluster - 2) * SPC
            img[cluster_offset * SPB:cluster_offset * SPB + len(chunk)] = chunk
            pos += len(chunk)

            # Mark FAT entry
            next_cluster = cluster + 1 if pos < file_size else 0x0FFFFFFF
            fat_offset = cluster * 4
            struct.pack_into('<I', fat_data, fat_offset, next_cluster)
            cluster += 1

        next_free_cluster = cluster
        file_clusters[name] = (start_cluster, file_size)

    # Write root directory cluster (cluster 2)
    root_offset = (data_start + (root_cluster - 2) * SPC) * SPB
    max_root_entries = SPC * SPB
    if len(dir_entries) > max_root_entries:
        print(f"Warning: need multiple clusters for root dir", file=sys.stderr)
    dir_data = bytearray(SPC * SPB)
    dir_data[:len(dir_entries)] = dir_entries
    img[root_offset:root_offset + SPC * SPB] = dir_data

    # Mark used clusters in FAT (all clusters after last written one are free)
    # FAT already has EOC for root and each file chain

    # Write FAT #1
    img[fat1_offset:fat1_offset + fat_size_bytes] = fat_data

    # Write FAT #2 (duplicate)
    fat2_offset = (RSVD + fat_sectors) * SPB
    img[fat2_offset:fat2_offset + fat_size_bytes] = fat_data

    # Write image
    with open(out_path, 'wb') as f:
        f.write(img)

    # Print summary
    print(f"FAT32 image: {out_path}")
    print(f"  Total sectors: {total_sectors} ({total_sectors * SPB / 1024 / 1024:.0f} MB)")
    print(f"  FAT sectors: {fat_sectors}")
    print(f"  Data start: LBA {data_start}")
    print(f"  Data clusters: {data_clusters}")
    for name, (cluster, size) in file_clusters.items():
        print(f"  File '{name}': cluster {cluster}, size {size} bytes")

if __name__ == '__main__':
    import json
    with open(sys.argv[1]) as f:
        config = json.load(f)
    make_fat32_image(config['total_sectors'], config['volume_label'],
                     [(f['name'], bytes.fromhex(f['data_hex'])) for f in config['files']],
                     config['out_path'])
