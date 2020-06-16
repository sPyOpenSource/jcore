# overall disk image size in megabytes (128M)
DISKSIZE=128
# boot partition size in kilobytes (16M)
BOOTSIZE=16
# boot partition FAT type (16 or 32). Note that smallest possible FAT32 is 33M
BOOTTYPE=16
# platform, either x86 or rpi
PLATFORM=x86
#PLATFORM=rpi
# the path to OVMF.fd (for testing with EFI)
OVMF=/usr/share/qemu/bios-TianoCoreEFI.bin

all: mkimg initrd.bin bootpart.bin disk

# compile the image creator
mkimg: mkimg.c
	gcc -ansi -pedantic -Wall -Wextra -g mkimg.c -o mkimg

# create an initial ram disk image with the kernel inside
initrd.bin:
	@mkdir initrd initrd/sys 2>/dev/null | true
ifeq ($(PLATFORM),x86)
	cp ../mykernel-rust/mykernel.x86_64.elf initrd/sys/core
else
	cp ../mykernel-rust/mykernel.aarch64.elf initrd/sys/core
endif
	@cd initrd && (find . | cpio -H hpodc -o | gzip > ../initrd.bin) && cd ..
	@rm -r initrd 2>/dev/null || true

# assemble the boot partition
bootpart.bin: initrd.bin
	@mkdir boot boot/BOOTBOOT 2>/dev/null || true
ifeq ($(PLATFORM),x86)
	@cp ../bootboot.bin boot/BOOTBOOT/LOADER || true
	@mkdir boot/EFI boot/EFI/BOOT
	@cp ../bootboot.efi boot/EFI/BOOT/BOOTX64.EFI || true
else
	@cp ../aarch64-rpi/LIC* ../aarch64-rpi/*.bin ../aarch64-rpi/*.elf ../aarch64-rpi/*.dat boot/ || true
	@cp ../bootboot.img boot/kernel8.img || true
endif
	@printf "screen=800x600\nkernel=sys/core\n" >boot/BOOTBOOT/CONFIG || true
	@cp initrd.bin boot/BOOTBOOT/INITRD || true
	./mkimg fat$(BOOTTYPE) $(BOOTSIZE) boot
	@rm -rf boot

# create hybrid disk / cdrom image or ROM image
disk: mkimg bootpart.bin
	./mkimg disk $(DISKSIZE) disk-$(PLATFORM).img

initrd.rom: mkimg initrd.bin
	./mkimg rom

# create a GRUB cdrom
grub.iso: initrd.bin
	@mkdir iso iso/bootboot iso/boot iso/boot/grub 2>/dev/null || true
	@cp ../bootboot.bin iso/bootboot/loader || true
	@printf "screen=800x600\nkernel=sys/core\n" >iso/bootboot/config || true
	@cp initrd.bin iso/bootboot/initrd || true
	@printf "menuentry \"BOOTBOOT test\" {\n  multiboot /bootboot/loader\n  module /bootboot/initrd\n  module /bootboot/config\n  boot\n}" >iso/boot/grub/grub.cfg || true
	grub-mkrescue -o grub.iso iso
	@rm -r iso 2>/dev/null || true

# test the disk image
rom: initrd.rom
	qemu-system-x86_64 -option-rom ../bootboot.bin -option-rom initrd.rom -serial stdio

bios:
	qemu-system-x86_64 -drive file=disk-x86.img,format=raw -serial stdio

cdrom:
	qemu-system-x86_64 -cdrom disk-x86.img -serial stdio

grubcdrom: grub.iso
	qemu-system-x86_64 -cdrom grub.iso -serial stdio

efi:
	qemu-system-x86_64 -bios $(OVMF) -m 64 -drive file=disk-x86.img,format=raw -serial stdio

eficdrom:
	qemu-system-x86_64 -bios $(OVMF) -m 64 -cdrom disk-x86.img -serial stdio

linux:
	qemu-system-x86_64 -kernel ../bootboot.bin -drive file=disk-x86.img,format=raw -serial stdio

sdcard:
	qemu-system-aarch64 -M raspi3 -kernel ../bootboot.img -drive file=disk-rpi.img,if=sd,format=raw -serial stdio

# clean up
clean:
	rm mkimg *.bin *.img *.rom 2>/dev/null || true
