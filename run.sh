#building the iso file
#mkdir -p Compiler/app/isodir/boot/grub
#make
#strip jxcore ; cp jxcore Compiler/app/isodir/jxcore
cd Compiler
ant clean run
cd app/isodir/code
zip -0 uncompressed.zip *.jll
cat uncompressed.zip | gzip --fast -c > ../code.zip
cd ../..

# Build a dual-boot ISO with matching BIOS and UEFI GRUB modules. The checked-in
# x86_64-efi modules are older than the installed x86_64-elf GRUB, so generate a
# fresh EFI image (and use its matching modules) in a staging tree first.
GRUB_LIB="$(brew --prefix x86_64-elf-grub)/lib/x86_64-elf/grub"
STAGE="$(mktemp -d)"
cp -R isodir "$STAGE/isodir"
rm -rf "$STAGE/isodir/boot/grub/x86_64-efi"
x86_64-elf-grub-mkrescue -o "$STAGE/efi-only.iso" "$STAGE/isodir"
xorriso -indev "$STAGE/efi-only.iso" -osirrox on -extract /efi.img "$STAGE/isodir/efi.img"
cp -R "$GRUB_LIB/x86_64-efi" "$STAGE/isodir/boot/grub/"
xorriso -as mkisofs -V ISOIMAGE -o MyOS.iso \
    -b boot/grub/i386-pc/eltorito.img -no-emul-boot -boot-load-size 4 -boot-info-table \
    -eltorito-alt-boot -e efi.img -no-emul-boot "$STAGE/isodir"
rm -rf "$STAGE"

#run it in qemu
# NOTE: the kernel sizes its heap from the multiboot memory map; QEMU's default
# of 128MB RAM is not enough and the kernel dies in an assertion loop (black screen).
# NOTE: -boot d, because the kernel itself lives on the ISO while disk.img only
# holds the data partition.
qemu-system-x86_64 -m 1024 -drive file=/Users/xuyi/Downloads/OVMF-X64-r15214/bbai-sd.img,format=raw -cdrom MyOS.iso -boot d -serial stdio
