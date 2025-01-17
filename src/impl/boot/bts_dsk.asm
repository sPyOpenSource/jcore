INITSTACK EQU 00009BFFCh ;Stack in protected mode starts here
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
BITS 32
section .text
; *** variables for primary image
PICDAddr: dd 0 ;class-descriptor for pi
PIExAddr: dd 0 ;method-offset for pi
; *** prepared gdt
ALIGN 4, db 000h ;align gdt to 4-byte-address
mygdt:
 dw 00000h, 00000h, 00000h, 00000h ;null descriptor
 dw 0FFFFh, 00000h, 09A00h, 000CFh ;4GB 32-bit code at 0x0
 dw 0FFFFh, 00000h, 09200h, 000CFh ;4GB 32-bit R/W at 0x0 (cf)
 dw 0FFFFh, 00000h, 09A06h, 0008Fh ;4GB 16-bit code at 0x60000
endgdt:
ptrgdt: ;use this offset for lgdt
 dw endgdt-mygdt ;length
 dd mygdt ;linear physical address (segment is 0)
PrintChar: ;print character in al, destroys es
 push si ;save si (destroyed by BIOS)
 push di ;save di (destroyed by BIOS)
 push bx ;save bx (destroyed for color)
 mov bl,007h ;color 007h: gray on black
 mov ah,00Eh ;function 00Eh: print character
 int 10h ;BIOS-call: graphics adapter
 pop bx ;restore bx
 pop di ;restore di
 pop si ;restore si
 ret ;return to caller
waitKeyboard: ;wait until inputbuffer empty
 in al,064h ;read status register
 test al,002h ;inputbuffer empty?
 jnz waitKeyboard ; no->retry
 ret
start2:
;________________ Initialize stack and ds ___________________________
 cli ;disble interrupts while setting ss:sp
 xor ax,ax ;helper for ss, ds, es
 mov ss,ax ;initialize stack
 mov sp,07BFCh ;highest unused byte
 mov ds,ax ;our address segment
;________________ Enable A20 gate ___________________________________
 mov al,'A' ;character to print
 call PrintChar ;say "A20"
 call waitKeyboard
 mov al,0D1h ;command: write output
 out 064h,al ;write command to command register
 call waitKeyboard
 mov al,0DFh ;everything to normal (A20 then enabled)
 out 060h,al ;write new value
 call waitKeyboard
;________________ Switch to protected and back to real mode _________
;now to protected mode (interrupts still cleared!)
 lgdt [ptrgdt] ;load gdt (pointer to six-byte-mem-loc)
 mov eax,cr0 ;read machine-status
 or al,1 ;set bit: protected mode enabled
 mov cr0,eax ;write machine-status
 mov dx,010h ;helper for segment registers
 mov fs,dx ;prepare fs for big flat segment model
 dec ax ;clear lowest bit: protected mode disabled
 mov cr0,eax ;write machine-status
;now back with large segment-limits
;________________ Switch to protected mode and call java ____________
CallJava:
 mov al,'P' ;character to print
 call PrintChar ;say "Protected Mode"
 lgdt [ptrgdt] ;load gdt (pointer to six-byte-mem-loc)
 mov eax,cr0 ;read machine-status
 or al,1 ;set bit: protected mode enabled
 mov cr0,eax ;write machine-status
 db 0EAh ;jump to 32-Bit Code
 dw doit ; offset (linear physical address)
 dw 008h ; selector
;________________ Initialise segments ____________________________
doit:
 mov dx,010h ;helper for data-segment
 mov ds,dx ;load data-segment into ds
 mov es,dx ;load data-segment into es
 mov fs,dx ;load data-segment into fs
 mov gs,dx ;load data-segment into gs
 mov ss,dx ;load data-segment into ss
 mov esp,INITSTACK ;set stackpointer
 mov edi,[PICDAddr] ;load address of class descriptor
 mov eax,[PIExAddr] ;load method address
 call eax ;call java-method
Ready:
 jmp Ready ;endless loop
end
