/*
 * mykernel-go/kernel.go
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief A sample BOOTBOOT compatible kernel
 *
 */

package kernel

import "unsafe"

/* Go sucks big time:
 * 1. no include (WTF they dare to call this a C killer language without a pre-compiler?)
 * 2. no union support nor pre-compiler conditionals for the arch specific struct part (no comment...)
 * 3. as soon as you declare a type struct (not use it, just declare) go will generate tons of unresolved runtime references
 * 4. importing font from another object file? Forget it... neither CGO nor .syso work for non-function labels
 * 5. even the "official" bare-metal-gophers example on github miserably fails to compile with the latest go compiler...
 * 6. if you finally manage to compile it, the resulting file is going to be twice the size of the C version!
 *
 * So we do dirty hacks here, only using pointers to access bootboot struct members, addresses by constants
 * instead of linker provided labels, plus no Hello World text, just cross-hair and R, G, B boxes.
 */

const (
    BOOTBOOT_MMIO = 0xfffffffff8000000  /* memory mapped IO virtual address */
    BOOTBOOT_FB   = 0xfffffffffc000000  /* frame buffer virtual address */
    BOOTBOOT_INFO = 0xffffffffffe00000  /* bootboot struct virtual address */
    BOOTBOOT_ENV  = 0xffffffffffe01000  /* environment string virtual address */
    BOOTBOOT_CORE = 0xffffffffffe02000  /* core loadable segment start */
)

/******************************************
 * Entry point, called by BOOTBOOT Loader *
 ******************************************/
func init() {
    /*** NOTE: this code runs on all cores in parallel ***/
    var x, y int
    var w int = (int)(*(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_INFO + 0x34))))
    var h int = (int)(*(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_INFO + 0x38))))
    var s int = (int)(*(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_INFO + 0x3c))))

    // cross-hair to see screen dimension detected correctly
    for y = 0; y < h; y++ {
        *(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_FB) + uintptr(s*y + w*2))) = 0x00FFFFFF;
    }
    for x = 0; x < w; x++ {
        *(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_FB) + uintptr(s*y/2 + x*4))) = 0x00FFFFFF;
    }

    // red, green, blue boxes in order
    for y = 0; y < 20; y++ {
        for x = 0; x < 20; x++ {
            *(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_FB) + uintptr(s*(y+20) + (x+20)*4))) = 0x00FF0000;
        }
    }
    for y = 0; y < 20; y++ {
        for x = 0; x < 20; x++ {
            *(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_FB) + uintptr(s*(y+20) + (x+50)*4))) = 0x0000FF00;
        }
    }
    for y = 0; y < 20; y++ {
        for x = 0; x < 20; x++ {
            *(*uint32)(unsafe.Pointer(uintptr(BOOTBOOT_FB) + uintptr(s*(y+20) + (x+80)*4))) = 0x000000FF;
        }
    }

    // in lack of a font, we can't say Hi

    // hang for now
    for ; ; {
    }
}
