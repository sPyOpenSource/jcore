BITS 32
section .multiboot_header
begin:
 ; magic number
 dd 0xe85250d6 ; multiboot2
 ; architecture
 dd 0 ; protected mode i386
 ; header length
 dd header_end - begin
 ; checksum
 dd 0x100000000 - (0xe85250d6 + 0 + (header_end - begin))
 ; end tag
 dw 0
 dw 0
 dd 8
header_end:
