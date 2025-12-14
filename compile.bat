#clang -std=c17 -target aarch64-unknown-custom -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c kernel/testfile.c -o build/testfile.o
#clang -std=c17 -target x86_64-unknown-custom -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c kernel/testfile.c -o build/testfile.o

#lld -flavor ld --oformat binary -o testfile.bin build/testfile.o

#clang -std=c17 -target aarch64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c loader/efi_main.c -o build/efi_main.o -Iinclude -Isrc
#clang -std=c17 -target aarch64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c loader/load.c -o build/load.o -Iinclude -Isrc
clang -std=c17 -target x86_64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c loader/efi_main.c -o build/efi_main.o -Iinclude -Isrc
clang -std=c17 -target x86_64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c loader/load.c -o build/load.o -Iinclude -Isrc
clang -std=c17 -target x86_64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c src/interface/zero_object.c -o build/zero_object.o -Iinclude -Isrc
clang -std=c17 -target x86_64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c src/interface/zero_DebugChannel.c -o build/zero_DebugChannel.o -Iinclude -Isrc
clang -std=c17 -target x86_64-unknown-windows -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -c src/interface/zero.c -o build/zero.o -Iinclude -Isrc

lld -flavor link -subsystem:efi_application -entry:efi_main build/efi_main.o build/load.o build/zero_object.o build/zero_DebugChannel.o build/zero.o -out:BOOTAA64.EFI
