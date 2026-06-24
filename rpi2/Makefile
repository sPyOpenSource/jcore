CROSS ?= /Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-
CC = clang --target=arm-none-eabi
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
CFLAGS = -mcpu=arm1176jzf-s -nostdlib -ffreestanding -O2
LDFLAGS = -marmelf -T linker.ld

all: kernel7-uart.img

kernel7-uart.img: boot_fixed.o kernel.o
	$(LD) $(LDFLAGS) $^ -o kernel.elf
	$(OBJCOPY) -O binary kernel.elf $@

boot_fixed.o: boot_fixed.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

qemu: kernel7-uart.img
	qemu-system-aarch64 -M raspi2b -device loader,file=$<,addr=0x8000,force-raw=on -serial stdio -display none

clean:
	rm -f *.o *.elf *.img
