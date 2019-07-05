#building the iso file
mkdir -p isodir/boot/grub
make jxcore
strip jxcore ; cat jxcore | gzip -9 -c > isodir/jxcore
grub-mkrescue -o MyOS.iso isodir

#run it in qemu
#qemu-system-x86_64 -cdrom MyOS.iso
