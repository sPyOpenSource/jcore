#!/bin/sh
#
# Create the boot disk image used by run.sh.
#
# The OS needs a real disk, not just an ISO:
#   * AIMemory starts bioide.Main with partition index 1, so the MBR has to
#     contain at least one primary partition (an empty MBR only yields the
#     whole-disk entry 0 -> IndexOutOfBoundsException).
#   * FSDomain mounts a FAT filesystem (jfat), so the partition needs a
#     FAT boot sector that Fat.create()/BootSector can parse.
#
# Layout: 64 MiB, one primary partition type 0x0B starting at sector 2048
# (1 MiB aligned), FAT16 volume label JXCORE.
#
# Usage: ./makedisk.sh [image]   (default: disk.img)
set -e

IMG="${1:-disk.img}"
SECTOR=512
START=2048                              # first sector of the partition
TOTAL_SECTORS=$((64 * 1024 * 1024 / SECTOR))
PART_SECTORS=$((TOTAL_SECTORS - START))

command -v python3 >/dev/null 2>&1 || { echo "makedisk: python3 not found" >&2; exit 1; }
if command -v mkfs.fat >/dev/null 2>&1; then
    MKFS="mkfs.fat -F 16 -n JXCORE"
elif command -v newfs_msdos >/dev/null 2>&1; then
    MKFS="newfs_msdos -F 16 -v JXCORE"
else
    echo "makedisk: neither mkfs.fat nor newfs_msdos found" >&2
    exit 1
fi

TMPDIR_=$(mktemp -d)
trap 'rm -rf "$TMPDIR_"' EXIT

# 1. blank disk
dd if=/dev/zero of="$IMG" bs=$SECTOR count=$TOTAL_SECTORS status=none

# 2. MBR: one bootable-in-DOS primary partition entry + 0x55AA signature
START=$START PART_SECTORS=$PART_SECTORS IMG="$IMG" python3 - <<'EOF'
import os, struct
start = int(os.environ["START"])
count = int(os.environ["PART_SECTORS"])
mbr = bytearray(512)
# status, CHS begin, type, CHS end, LBA start, LBA length
mbr[446:462] = struct.pack("<BBBBBBBBII", 0x00, 0xFE, 0xFF, 0xFF,
                           0x0B, 0xFE, 0xFF, 0xFF, start, count)
mbr[510:512] = b"\x55\xAA"
with open(os.environ["IMG"], "r+b") as f:
    f.seek(446)
    f.write(mbr[446:512])
EOF

# 3. format the partition, then splice it into the image at its LBA offset
dd if=/dev/zero of="$TMPDIR_/part.img" bs=$SECTOR count=$PART_SECTORS status=none
$MKFS "$TMPDIR_/part.img"
dd if="$TMPDIR_/part.img" of="$IMG" bs=$SECTOR seek=$START conv=notrunc status=none

echo "makedisk: wrote $IMG ($((TOTAL_SECTORS / 2048)) MiB, partition at sector $START, $PART_SECTORS sectors, FAT16)"
