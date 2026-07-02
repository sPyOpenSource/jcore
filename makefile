# sudo apt-get install g++ binutils libc6-dev-amd64
# sudo apt-get install VirtualBox grub-pc xorriso
# macOS: brew install llvm (provides ld.lld)

CC = clang
LD := $(shell command -v /opt/homebrew/bin/ld.lld 2>/dev/null || command -v ld.lld 2>/dev/null || echo "ld.lld")
GRUB_MKRESCUE := $(shell command -v i686-elf-grub-mkrescue 2>/dev/null || command -v grub-mkrescue 2>/dev/null || echo "grub-mkrescue")

TARGET = --target=x86_64-pc-none-elf
GCCPARAMS = $(TARGET) -ffreestanding -Isrc -I FlintJVM/Inc -I FlintJVM/Native/Inc -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -Wno-write-strings
ASPARAMS = $(TARGET)
LDPARAMS = -melf_x86_64

objects = obj/boot32.o \
		  obj/boot64.o \
          obj/cstandardlib.o \
          obj/gdt.o \
          obj/memorymanagement.o \
          obj/drivers/driver.o \
          obj/hardwarecommunication/port.o \
          obj/hardwarecommunication/interruptstubs.o \
          obj/hardwarecommunication/interrupts.o \
          obj/syscalls.o \
          obj/multitasking.o \
          obj/drivers/amd_am79c973.o \
          obj/hardwarecommunication/pci.o \
          obj/drivers/keyboard.o \
          obj/drivers/mouse.o \
          obj/drivers/vga.o \
          obj/drivers/ata.o \
          obj/filesystem/msdospart.o \
          obj/filesystem/fat.o \
          obj/filesystem/SdFile.o \
          obj/filesystem/Volume.o \
          obj/gui/widget.o \
          obj/gui/window.o \
          obj/gui/desktop.o \
          obj/net/etherframe.o \
          obj/net/arp.o \
          obj/net/ipv4.o \
          obj/net/icmp.o \
          obj/net/udp.o \
          obj/net/tcp.o \
           obj/kernel.o \
           obj/flint.o \
           obj/flint_array_object.o \
           obj/flint_class_loader.o \
           obj/flint_common.o \
           obj/flint_const_pool.o \
           obj/flint_debugger.o \
           obj/flint_execution.o \
           obj/flint_field_info.o \
           obj/flint_fields_data.o \
           obj/flint_java_class.o \
           obj/flint_java_class_dict_node.o \
           obj/flint_java_object.o \
           obj/flint_java_string.o \
           obj/flint_java_string_dict_node.o \
           obj/flint_java_thread.o \
           obj/flint_java_throwable.o \
           obj/flint_method_info.o \
           obj/flint_mutex.o \
           obj/flint_utf8.o \
           obj/flint_utf8_dict_node.o \
           obj/flint_native.o \
           obj/flint_native_array.o \
           obj/flint_native_character.o \
           obj/flint_native_class.o \
           obj/flint_native_constructor.o \
           obj/flint_native_double.o \
           obj/flint_native_float.o \
           obj/flint_native_interface.o \
           obj/flint_native_io_file.o \
           obj/flint_native_io_file_descriptor.o \
           obj/flint_native_io_file_input_stream.o \
            obj/flint_native_io_file_output_stream.o \
            obj/flint_native_math_stubs.o \
            obj/opencode_stubs.o \
            obj/flint_native_method.o \
           obj/flint_native_object.o \
           obj/flint_native_reflection.o \
           obj/flint_native_string.o \
           obj/flint_native_system.o \
           obj/flint_native_thread.o \
           obj/jvm/jvm_bridge.o

run: mykernel.iso
	@if command -v VirtualBox >/dev/null 2>&1 && VirtualBox --version >/dev/null 2>&1; then \
		echo "Starting with VirtualBox..."; \
		(killall VirtualBox && sleep 1) || true; \
		VirtualBox --startvm 'kayos' & \
	elif command -v qemu-system-x86_64 >/dev/null 2>&1; then \
		echo "VirtualBox not found or broken, starting with QEMU..."; \
		qemu-system-x86_64 -cdrom mykernel.iso -m 128 -boot d; \
	else \
		echo "Error: Neither VirtualBox nor QEMU found. Please install one of them."; \
		exit 1; \
	fi

obj/flint.o: FlintJVM/flint.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -c -o $@ $<

obj/flint_%.o: FlintJVM/flint_%.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -c -o $@ $<

obj/flint_native.o: FlintJVM/Native/flint_native.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -I FlintJVM/Native/Inc -c -o $@ $<

obj/flint_native_%.o: FlintJVM/Native/flint_native_%.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -I FlintJVM/Native/Inc -c -o $@ $<

obj/jvm/jvm_bridge.o: src/jvm/jvm_bridge.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -I FlintJVM/Native/Inc -c -o $@ $<

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	$(CC) $(ASPARAMS) -c -o $@ $<

mykernel.bin: linker.ld $(objects)
	$(LD) $(LDPARAMS) -T $< -o $@ $(objects)

mykernel.iso: mykernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp mykernel.bin iso/boot/mykernel.bin
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot2 /boot/mykernel.bin'   >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) --output=mykernel.iso iso
	rm -rf iso

install: mykernel.bin
	sudo cp $< /boot/mykernel.bin

.PHONY: clean
clean:
	rm -rf obj mykernel.bin mykernel.iso
