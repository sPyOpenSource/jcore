# Toolchain
CC = clang --target=arm-none-eabi
ARMGNU_OBJCOPY = /Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-objcopy

# Flags
CFLAGS = -Wall -Werror -O2 -nostdlib -ffreestanding -marm -mabi=aapcs
LDFLAGS = -nostartfiles -nostdlib -fuse-ld=lld

.PHONY: all clean run-qemu

all: kernel.bin

# --- Real hardware (BeagleBone AI) ---

boot.o: src/boot.asm
	$(CC) -c $< -o $@

main.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: boot.o main.o linker.ld
	$(CC) $(LDFLAGS) -Wl,-T,linker.ld boot.o main.o -o $@

kernel.bin: kernel.elf
	$(ARMGNU_OBJCOPY) -O binary $< $@

# --- QEMU (ARM Virt) ---

boot-qemu.o: src/boot.asm
	$(CC) -c $< -o $@

main-qemu.o: src/main.c
	$(CC) $(CFLAGS) -DQEMU -c $< -o $@

kernel-qemu.elf: boot-qemu.o main-qemu.o linker-qemu.ld
	$(CC) $(LDFLAGS) -Wl,-T,linker-qemu.ld boot-qemu.o main-qemu.o -o $@

run-qemu: kernel-qemu.elf
	qemu-system-arm -M virt -cpu cortex-a15 -nographic -kernel $<

# --- Clean ---

clean:
	rm -f *.o *.elf *.bin
