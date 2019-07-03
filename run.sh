#building the iso file
mkdir -p isodir/boot/grub
strip jxcore ; cat jxcore | gzip -9 -c > isodir/jxcore
#cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o MyOS.iso isodir

#run it in qemu
qemu-system-x86_64 -cdrom MyOS.iso
