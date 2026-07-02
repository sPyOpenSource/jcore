CC = clang --target=arm-none-eabi
ARMGNU_OBJCOPY = /Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-objcopy

CFLAGS = -Wall -Werror -O2 -nostdlib -ffreestanding -marm -mabi=aapcs \
         -mcpu=cortex-a15 -mfpu=neon -mfloat-abi=hard
AFLAGS = -marm -mcpu=cortex-a15 -mfpu=neon -mfloat-abi=hard
LDFLAGS = -nostartfiles -nostdlib -fuse-ld=lld

JCORE_WARN = -Wall -Wno-deprecated-non-prototype -Wno-visibility -Wno-unused-variable -Wno-implicit-function-declaration -Wno-int-conversion -Wno-unused-function -Wno-shift-count-overflow
JCORE_CFLAGS = $(JCORE_WARN) -O2 -nostdlib -ffreestanding -marm -mabi=aapcs -mcpu=cortex-a15 \
               -DKERNEL -DTIMER_HZ=10 -DCPU_MHZ=1000 -Isrc/jcore -Ijcore -I.
JCORE_AFLAGS = $(AFLAGS) -DKERNEL -DTIMER_HZ=10 -DCPU_MHZ=1000 -Isrc/jcore -Ijcore -I. -DASSEMBLER

MODEL_DIR = models

.PHONY: all clean run-qemu run-jcore-qemu

all: kernel.bin

# === jcore kernel ===

# ARM layer objects
JCORE_ARM_OBJS = \
	start-jcore.o \
	switch_asm-jcore.o \
	serial-jcore.o \
	gic-jcore.o \
	timer-jcore.o \
	exception-jcore.o \
	main-jcore.o

# jcore library objects
JCORE_LIB_OBJS = \
	thread-jcore.o \
	runq-jcore.o \
	minic-jcore.o

$(JCORE_ARM_OBJS) $(JCORE_LIB_OBJS): Makefile

start-jcore.o: src/jcore/start.S
	$(CC) -c $(JCORE_AFLAGS) $< -o $@

switch_asm-jcore.o: src/jcore/switch_asm.S
	$(CC) -c $(JCORE_AFLAGS) $< -o $@

serial-jcore.o: src/jcore/serial.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

gic-jcore.o: src/jcore/gic.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

timer-jcore.o: src/jcore/timer.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

exception-jcore.o: src/jcore/exception.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

main-jcore.o: src/jcore/main.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

thread-jcore.o: jcore/thread.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

runq-jcore.o: jcore/runq.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

minic-jcore.o: jcore/minic.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

misc-jcore.o: jcore/misc.c
	$(CC) $(JCORE_CFLAGS) -c $< -o $@

JCORE_OBJS = $(JCORE_ARM_OBJS) $(JCORE_LIB_OBJS)

kernel-jcore.elf: $(JCORE_OBJS) linker-jcore.ld
	$(CC) $(LDFLAGS) -Wl,-T,linker-jcore.ld $(filter %.o,$^) -o $@

kernel-jcore.bin: kernel-jcore.elf
	$(ARMGNU_OBJCOPY) -O binary $< $@

run-jcore-qemu: kernel-jcore.elf
	qemu-system-arm -M virt -cpu cortex-a15 -m 128M -nographic -kernel $<

# === Legacy llama2 standalone ===

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

model_bin.o: model_bin.S
	$(CC) -c $(AFLAGS) $< -o $@

boot.o: src/boot.asm
	$(CC) -c $(AFLAGS) $< -o $@

boot-qemu.o: src/boot.asm
	$(CC) -c $(AFLAGS) $< -o $@

llama2.o: src/llama2.c src/llama2.h
	$(CC) $(CFLAGS) -c $< -o $@

llama2-qemu.o: src/llama2.c src/llama2.h
	$(CC) $(CFLAGS) -DQEMU -c $< -o $@

main.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

main-qemu.o: src/main.c
	$(CC) $(CFLAGS) -DQEMU -c $< -o $@

aeabi.o: src/aeabi.c
	$(CC) $(CFLAGS) -c $< -o $@

aeabi-qemu.o: src/aeabi.c
	$(CC) $(CFLAGS) -DQEMU -c $< -o $@

# --- BBAI target ---

kernel.elf: boot.o main.o llama2.o aeabi.o model_bin.o linker.ld
	$(CC) $(LDFLAGS) -Wl,-T,linker.ld $(filter %.o,$^) -o $@

kernel.bin: kernel.elf
	$(ARMGNU_OBJCOPY) -O binary $< $@

# --- QEMU target ---

kernel-qemu.elf: boot-qemu.o main-qemu.o llama2-qemu.o aeabi-qemu.o model_bin.o linker-qemu.ld
	$(CC) $(LDFLAGS) -Wl,-T,linker-qemu.ld $(filter %.o,$^) -o $@

run-qemu: kernel-qemu.elf
	qemu-system-arm -M virt -cpu cortex-a15 -m 128M -nographic -kernel $<

# --- Bootstrap ---

$(MODEL_DIR)/stories260K.bin:
	mkdir -p $(MODEL_DIR)
	curl -L -o $@ $(MODEL_260K_URL)

model-260K: $(MODEL_DIR)/stories260K.bin

# --- Clean ---

clean:
	rm -f *.o *.elf *.bin model_bin.S

MODEL_260K_URL = https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/stories260K.bin
