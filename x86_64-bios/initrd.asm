;*
;* x86_64-bios/initrd.asm
;*
;* Copyright (C) 2017 bzt (bztsrc@gitlab)
;*
;* Permission is hereby granted, free of charge, to any person
;* obtaining a copy of this software and associated documentation
;* files (the "Software"), to deal in the Software without
;* restriction, including without limitation the rights to use, copy,
;* modify, merge, publish, distribute, sublicense, and/or sell copies
;* of the Software, and to permit persons to whom the Software is
;* furnished to do so, subject to the following conditions:
;*
;* The above copyright notice and this permission notice shall be
;* included in all copies or substantial portions of the Software.
;*
;* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;* DEALINGS IN THE SOFTWARE.
;*
;* This file is part of the BOOTBOOT Protocol package.
;* @brief Option ROM for initrd. mkfs utility also can do this.
;*
;* This may seem odd, but works with BIOS (up to 96k gzipped
;* initrd), and makes much more sense on EFI, where Option ROMs
;* can be 16M in size.
;*

;------------header------------
            USE16
            ORG         0h
rom:        db          55h,0AAh                ;ROM magic
            db          (rom_end-rom)/512       ;size in 512 blocks
.executor:  xor         ax, ax                  ;entry point
            retf
.checksum:  dw          0                       ;checksum
.name:      db          "INITRD"
            db          0,0
            dd          initrd_end-initrd, 0
            db          0,0
.pnpptr:    dw          0
.flags:     dd          0
;------------data------------
initrd:
            file        "../../bin/ESP/BOOTBOOT/INITRD"
initrd_end:
;-----------padding to be multiple of 512----------
            db          (511-($-rom+511) mod 512) dup 0
rom_end:

;-----------BIOS checksum------------
chksum = 0
repeat $-rom
    load b byte from (rom+%-1)
    chksum = (chksum + b) mod 100h
end repeat
store byte (100h-chksum) at (rom.checksum)
