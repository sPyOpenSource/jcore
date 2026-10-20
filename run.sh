#building the iso file
mkdir -p Compiler/app/isodir/boot/grub
make
strip jxcore ; cp jxcore Compiler/app/isodir/jxcore
cd Compiler/app/isodir/code
zip -0 uncompressed.zip *.jll
cat uncompressed.zip | gzip --fast -c > ../code.zip
cd ../..
i686-elf-grub-mkrescue -o MyOS.iso isodir

#run it in VirtualBox
#(killall VirtualBox && sleep 1) || true
#VBoxManage startvm 'JavaOS' &

#boot disk (partitioned FAT volume) - the OS needs it, see makedisk.sh
if [ ! -f disk.img ]; then
    ./makedisk.sh disk.img
fi

#run it in qemu
# NOTE: the kernel sizes its heap from the multiboot memory map; QEMU's default
# of 128MB RAM is not enough and the kernel dies in an assertion loop (black screen).
# NOTE: -boot d, because the kernel itself lives on the ISO while disk.img only
# holds the data partition.
qemu-system-x86_64 -m 1024 -drive file=disk.img,format=raw -cdrom MyOS.iso -boot d -serial stdio
