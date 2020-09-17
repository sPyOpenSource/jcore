# platform, either x86 or rpi
PLATFORM=x86
#PLATFORM=rpi
# the path to OVMF.fd (for testing with EFI)
OVMF=/usr/share/qemu/bios-TianoCoreEFI.bin

all: ../mkbootimg/mkbootimg initdir disk

# compile the image creator
../mkbootimg/mkbootimg:
	@make -C ../mkbootimg all

# create an initial ram disk image with the kernel inside
initdir:
	@mkdir initrd initrd/sys 2>/dev/null | true
ifeq ($(PLATFORM),x86)
	cp ../mykernel/mykernel.x86_64.elf initrd/sys/core
else
	cp ../mykernel/mykernel.aarch64.elf initrd/sys/core
endif

# create hybrid disk / cdrom image or ROM image
disk: ../mkbootimg/mkbootimg initdir mkbootimg.json
	../mkbootimg/mkbootimg mkbootimg.json disk-$(PLATFORM).img
	@rm -rf initrd

initrd.rom: ../mkbootimg/mkbootimg initdir mkbootimg.json
	../mkbootimg/mkbootimg mkbootimg.json initrd.rom
	@rm -rf initrd

# create a GRUB cdrom
grub.iso: ../mkbootimg/mkbootimg initdir mkbootimg.json
	@../mkbootimg/mkbootimg mkbootimg.json initrd.bin
	@rm -rf initrd
	@mkdir iso iso/bootboot iso/boot iso/boot/grub 2>/dev/null || true
	@cp ../bootboot.bin iso/bootboot/loader || true
	@cp config iso/bootboot/config || true
	@cp initrd.bin iso/bootboot/initrd || true
	@printf "menuentry \"BOOTBOOT test\" {\n  multiboot /bootboot/loader\n  module /bootboot/initrd\n  module /bootboot/config\n  boot\n}" >iso/boot/grub/grub.cfg || true
	grub-mkrescue -o grub.iso iso
	@rm -r iso 2>/dev/null || true

# test the disk image
rom: initrd.rom
	qemu-system-x86_64 -option-rom ../dist/bootboot.bin -option-rom initrd.rom -serial stdio

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
	qemu-system-x86_64 -kernel ../dist/bootboot.bin -drive file=disk-x86.img,format=raw -serial stdio

sdcard:
	qemu-system-aarch64 -M raspi3 -kernel ../dist/bootboot.img -drive file=disk-rpi.img,if=sd,format=raw -serial stdio

coreboot:
	qemu-system-x86_64 -bios coreboot.rom -drive file=disk-x86.img,format=raw -serial stdio

# clean up
clean:
	rm -rf initrd *.bin *.img *.rom *.iso 2>/dev/null || true
