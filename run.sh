#building the iso file
#mkdir -p isodir/boot/grub
#make
#strip jxcore ; cat jxcore | gzip -9 -c > isodir/jxcore
cd Compiler/app/isodir/code
zip -0 uncompressed.zip *.jll
cat uncompressed.zip | gzip --fast -c > ../code.zip
cd ../..
grub-mkrescue -o MyOS.iso isodir

#run it in VirtualBox
#(killall VirtualBox && sleep 1) || true
#VBoxManage startvm 'JavaOS' &

#run it in qemu
qemu-system-x86_64 -cdrom MyOS.iso
