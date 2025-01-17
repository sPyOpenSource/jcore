
x86_64_c_source_files := src/impl/print.c src/impl/main.c
x86_64_c_object_files := build/x86_64/print.o build/x86_64/main.o

x86_64_asm_source_files := $(shell find src/impl/boot -name *.asm)
x86_64_asm_object_files := $(patsubst src/impl/boot/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

$(x86_64_c_object_files):
	mkdir -p $(dir $@) && \
	gcc -c -I src/includes -ffreestanding $(patsubst build/x86_64/%.o, src/impl/%.c, $@) -o $@

$(x86_64_asm_object_files): build/x86_64/%.o : src/impl/boot/%.asm
	mkdir -p $(dir $@) && \
	nasm -f elf64 $(patsubst build/x86_64/%.o, src/impl/boot/%.asm, $@) -o $@

.PHONY: build
all: $(x86_64_object_files)
	mkdir -p dist/x86_64 && \
	ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(x86_64_object_files) && \
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin && \
	grub-mkrescue -o dist/x86_64/kernel.iso targets/x86_64/iso
