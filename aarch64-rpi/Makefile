all: boot.S bootboot.c headers/fs.h
	@echo "  src		aarch64-rpi (Raspberry Pi 3+)"
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-gcc -Wall -Wextra -pedantic -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -c boot.S -o build/boot.o
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-gcc -Wall -Wextra -pedantic -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -I ./headers -c tinflate.c -o build/tinflate.o
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-gcc -Wall -Wextra -pedantic -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -I ./headers -c bootboot.c -o build/bootboot.o
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-gcc -Wall -Wextra -pedantic -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -I ./headers -I ./ -c load.c -o build/load.o
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-ld -r -b binary -o build/font.o font.psf
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-ld -nostdlib build/boot.o build/bootboot.o build/tinflate.o build/font.o -T link.ld -o bootboot.elf
	@/Users/xuyi/opt/arm-gnu-toolchain-15.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-objcopy -O binary bootboot.elf ../dist/bootboot.img

mkboot: mkboot.c
	gcc mkboot.c -o mkboot

raspbootcom: raspbootcom.c
	gcc raspbootcom.c -o raspbootcom

getfw:
	wget -c -q https://raw.githubusercontent.com/raspberrypi/firmware/master/boot/LICENCE.broadcom
	wget -c -q https://raw.githubusercontent.com/raspberrypi/firmware/master/boot/bootcode.bin
	wget -c -q https://raw.githubusercontent.com/raspberrypi/firmware/master/boot/fixup.dat
	wget -c -q https://raw.githubusercontent.com/raspberrypi/firmware/master/boot/start.elf

cleanfw:
	@rm LICENCE.broadcom bootcode.bin fixup.dat start.elf

clean:
	@rm mkboot raspbootcom *.o >/dev/null 2>/dev/null || true
