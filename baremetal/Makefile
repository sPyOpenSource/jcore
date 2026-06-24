CROSS ?= /Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-
CC = clang --target=arm-none-eabi
CC64 = clang --target=aarch64-none-elf
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
CFLAGS = -mcpu=arm1176jzf-s -nostdlib -ffreestanding -O2
CFLAGS64 = -nostdlib -ffreestanding -O2 -DAARCH64
LDFLAGS = -marmelf -T linker.ld
LDFLAGS64 = -T linker64.ld

all: kernel7-uart.img kernel8.img kernel8-raspi4.img

kernel7-uart.img: boot_fixed.o kernel.o
	$(LD) $(LDFLAGS) $^ -o kernel.elf
	$(OBJCOPY) -O binary kernel.elf $@

kernel8.img: boot64.o kernel64.o
	$(LD) $(LDFLAGS64) $^ -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf $@

kernel8-raspi4.img: boot64.o kernel64-raspi4.o
	$(LD) $(LDFLAGS64) $^ -o kernel8-raspi4.elf
	$(OBJCOPY) -O binary kernel8-raspi4.elf $@

boot_fixed.o: boot_fixed.S
	$(CC) $(CFLAGS) -c $< -o $@

boot64.o: boot64.S
	$(CC64) $(CFLAGS64) -c $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel64.o: kernel.c
	$(CC64) $(CFLAGS64) -c $< -o $@

kernel64-raspi4.o: kernel.c
	$(CC64) $(CFLAGS64) -DRASPI_VER=4 -c $< -o $@

qemu: kernel7-uart.img
	qemu-system-aarch64 -M raspi2b -device loader,file=$<,addr=0x8000,force-raw=on -serial stdio -display none

qemu8: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -serial stdio -display none

qemu8-raspi4: kernel8-raspi4.img
	qemu-system-aarch64 -M raspi4b -kernel $< -serial stdio -display none

clean:
	rm -f *.o *.elf *.img
