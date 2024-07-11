clang -std=c17 -target aarch64-unknown-custom -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c kernel/testfile.c -o testfile.o

lld -flavor ld --oformat binary -o testfile.bin testfile.o

clang -std=c17 -target aarch64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c efi_main.c -o efi_main.o

lld -flavor link -subsystem:efi_application -entry:efi_main efi_main.o -out:BOOTAA64.EFI

del *.o
@echo on

pause