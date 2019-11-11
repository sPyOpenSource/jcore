#building the iso file
#mkdir -p isodir/boot/grub
make jxcore
strip jxcore ; cat jxcore | gzip -9 -c > isodir/jxcore
cd isodir/code
zip -0 uncompressed.zip *.jll
cat uncompressed.zip | gzip --fast -c > ../code.zip
cd ../..
grub-mkrescue -o MyOS.iso isodir
(killall VirtualBox && sleep 1) || true
VirtualBox --startvm 'JavaOS' &
#run it in qemu
#qemu-system-x86_64 -cdrom MyOS.iso
