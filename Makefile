CC = clang --target=arm-none-eabi
ARMGNU_OBJCOPY = /Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-objcopy

AFLAGS = -marm -mcpu=cortex-a15 -mfpu=neon -mfloat-abi=hard
LDFLAGS = -nostartfiles -nostdlib -fuse-ld=lld

JCORE_WARN = -Wall -Wno-deprecated-non-prototype -Wno-visibility -Wno-unused-variable -Wno-implicit-function-declaration -Wno-int-conversion -Wno-unused-function -Wno-shift-count-overflow
JCORE_CFLAGS = $(JCORE_WARN) -O2 -nostdlib -ffreestanding -marm -mabi=aapcs -mcpu=cortex-a15 \
               -DQEMU -DKERNEL -DTIMER_HZ=10 -DCPU_MHZ=1000 -Isrc/jcore/includes -Isrc/includes -Isrc -I.
JCORE_AFLAGS = $(AFLAGS) -DKERNEL -DTIMER_HZ=10 -DCPU_MHZ=1000 -Isrc/jcore/includes -I. -DASSEMBLER

MODEL_DIR = models

.PHONY: all clean run-jcore-qemu

all: kernel-jcore.bin

# === jcore kernel ===

JCORE_ARM_OBJS = \
	start-jcore.o \
	switch_asm-jcore.o \
	serial-jcore.o \
	gic-jcore.o \
	timer-jcore.o \
	exception-jcore.o \
	main-jcore.o \
	prcm-jcore.o \
	pinmux-jcore.o \
	mmc2-jcore.o \
	ram_blk-jcore.o \
	dss-jcore.o \
	hdmi-jcore.o

JCORE_LIB_OBJS = \
	minirun-jcore.o \
	sched-jcore.o \
	minic-jcore.o \
	llama2-jcore.o \
	llm-jcore.o \
	led_debug-jcore.o \
	aeabi-jcore.o \
	fat32-jcore.o \
	vfs-jcore.o \
	fat32_vfs-jcore.o

$(JCORE_ARM_OBJS) $(JCORE_LIB_OBJS): Makefile

start-jcore.o: src/start.S
	$(CC) -c $(JCORE_AFLAGS) $< -o $@

switch_asm-jcore.o: src/switch_asm.S
	$(CC) -c $(JCORE_AFLAGS) $< -o $@

serial-jcore.o: src/serial.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

gic-jcore.o: src/gic.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

timer-jcore.o: src/timer.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

exception-jcore.o: src/exception.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

main-jcore.o: src/main.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

prcm-jcore.o: src/drivers/prcm.c src/drivers/prcm.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

pinmux-jcore.o: src/drivers/pinmux.c src/drivers/pinmux.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

mmc2-jcore.o: src/drivers/mmc2.c src/drivers/mmc2.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

ram_blk-jcore.o: src/drivers/ram_blk.c src/drivers/ram_blk.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

dss-jcore.o: src/drivers/dss.c src/drivers/dss.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

hdmi-jcore.o: src/drivers/hdmi.c src/drivers/hdmi.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

minirun-jcore.o: src/minirun.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

sched-jcore.o: src/sched.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

minic-jcore.o: src/jcore/minic.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

llama2-jcore.o: src/llama2.c src/includes/llama2.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

llm-jcore.o: src/llm.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

led_debug-jcore.o: src/led_debug.c src/includes/led_debug.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

aeabi-jcore.o: src/aeabi.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

fat32-jcore.o: src/fs/fat32.c src/fs/fat32.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

vfs-jcore.o: src/fs/vfs.c src/fs/vfs.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

fat32_vfs-jcore.o: src/fs/fat32_vfs.c src/fs/fat32.h src/fs/vfs.h
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

model_bin-jcore.o: model_bin.S
	$(CC) -c $(JCORE_AFLAGS) $< -o $@

JCORE_OBJS = $(JCORE_ARM_OBJS) $(JCORE_LIB_OBJS) model_bin-jcore.o

kernel-jcore.elf: $(JCORE_OBJS) linker-jcore.ld
	$(CC) $(LDFLAGS) -Wl,-T,linker-jcore.ld $(filter %.o,$^) -o $@

kernel-jcore.bin: kernel-jcore.elf
	$(ARMGNU_OBJCOPY) -O binary $< $@

FAT32_IMG = models/fat32-test.img

run: kernel-jcore.elf $(FAT32_IMG)
	qemu-system-arm -M virt -cpu cortex-a15 -m 128M -nographic \
		-device loader,file=$(FAT32_IMG),addr=0x45000000,force-raw=on \
		-kernel $<

# === Model embedding ===

model.bin: $(MODEL_DIR)/stories260K.bin
	cp $< $@

model_bin.S: model.bin
	@echo '	.section .data' > $@
	@echo '	.balign 16' >> $@
	@echo '	.global _binary_model_bin_start' >> $@
	@echo '	.global _binary_model_bin_end' >> $@
	@echo '_binary_model_bin_start:' >> $@
	@echo '	.incbin "model.bin"' >> $@
	@echo '_binary_model_bin_end:' >> $@

# === Bootstrap ===

$(MODEL_DIR)/stories260K.bin:
	mkdir -p $(MODEL_DIR)
	curl -L -o $@ $(MODEL_260K_URL)

model-260K: $(MODEL_DIR)/stories260K.bin

# === Clean ===

clean:
	rm -f *.o *.elf *.bin model_bin.S

MODEL_260K_URL = https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/stories260K.bin
