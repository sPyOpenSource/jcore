# Toolchain
CC = clang --target=arm-none-eabi
ARMGNU_OBJCOPY = /Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-objcopy

# Flags
CFLAGS = -Wall -Werror -O2 -nostdlib -ffreestanding -marm -mabi=aapcs
LDFLAGS = -fuse-ld=lld -nostartfiles -nostdlib -Wl,-T,linker.ld

.PHONY: all clean

all: kernel.bin

boot.o: src/boot.asm
	$(CC) -c $< -o $@

main.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: boot.o main.o linker.ld
	$(CC) $(LDFLAGS) boot.o main.o -o $@

kernel.bin: kernel.elf
	$(ARMGNU_OBJCOPY) -O binary $< $@

clean:
	rm -f *.o *.elf *.bin
