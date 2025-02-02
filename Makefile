ARMGNU = arm-linux-gnueabi
 
AOPS = --warn --fatal-warnings
COPS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding 
 
boot.bin: boot.asm
	$(ARMGNU)-as src/boot.asm -o boot.o
	$(ARMGNU)-ld -T linker.ld boot.o -o boot.elf
	$(ARMGNU)-objdump -D boot.elf > boot.list
	$(ARMGNU)-objcopy boot.elf -O srec boot.srec
	$(ARMGNU)-objcopy boot.elf -O binary boot.bin
