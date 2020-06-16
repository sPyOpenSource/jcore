
all: mykernel.x86_64.elf

# Kernel build
mykernel.x86_64.elf: src/**
	objcopy -O elf64-x86-64 -B i386 -I binary font.psf font.o
	cargo xbuild --target ./triplets/mykernel-x86.json
	# Using this causes the kernel to exceed the 2 MB limit
	# cargo build -Z build-std=core,alloc --target ./triplets/$mykernel-x86.json
	cp ./target/mykernel-x86/debug/mykernel-rust mykernel.x86_64.elf