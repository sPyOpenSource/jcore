/*
 * mykernel/go/kernel.go
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

package main

import (
	_ "embed"
	"unsafe"

	. "gitlab.com/bztsrc/bootboot/bootboot"
)

var (
	//go:cgo_export_static bootboot bootboot
	//go:linkname bootboot bootboot
	bootboot BOOTBOOT

	//go:cgo_export_static fb fb
	//go:linkname fb fb
	fb uint32
)

type psf2 struct {
	magic        uint32
	version      uint32
	headerSize   uint32
	flags        uint32
	numGlyph     uint32
	bytePerGlyph uint32
	height       uint32
	width        uint32
	glyphs       uint8
}

//go:embed font.psf
var binaryFontPSF []byte

/******************************************
 * Entry point, called by BOOTBOOT Loader *
 ******************************************/
// Here is why each pragma below is needed:
//
// cgo_export_static - Exports the symbol for the external linker to see
// since Go will localize symbols by default.
//
// linkname - Go symbols are a combination of their package and func name.
// So this func is actually main._start This creates a link to this function
// just as _start so that the exported C name knows which functions to call.
//
// nosplit - Go has growable stacks. Since we haven't setup a proper stack yet
// disable the stack growth check in this function. Once the stack is setup
// change the runtime.g0 symbol to hold the size of it and place the pointer
// to it inside R14.
//
//go:cgo_export_static _start _start
//go:linkname _start _start
//go:nosplit
func _start() {
	/*** NOTE: this code runs on all cores in parallel ***/
	var x, y int
	w := int(bootboot.FbWidth)
	h := int(bootboot.FbHeight)
	s := int(bootboot.FbScanline)

	if s > 0 {
		// cross-hair to see screen dimension detected correctly
		for y = 0; y < h; y++ {
			*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(s*y+w*2))) = 0x00FFFFFF
		}
		for x = 0; x < w; x++ {
			*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(s*y/2+x*4))) = 0x00FFFFFF
		}

		// red, green, blue boxes in order
		for y = 0; y < 20; y++ {
			for x = 0; x < 20; x++ {
				*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(s*(y+20)+(x+20)*4))) = 0x00FF0000
			}
		}
		for y = 0; y < 20; y++ {
			for x = 0; x < 20; x++ {
				*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(s*(y+20)+(x+50)*4))) = 0x0000FF00
			}
		}
		for y = 0; y < 20; y++ {
			for x = 0; x < 20; x++ {
				*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(s*(y+20)+(x+80)*4))) = 0x000000FF
			}
		}

		// say hello
		puts("Hello from a simple BOOTBOOT kernel")
	}

	// hang for now
	for {
	}
}

/**************************
 * Display text on screen *
 **************************/
func puts(s string) {
	font := (*psf2)(unsafe.Pointer(&binaryFontPSF[0]))
	glyphs := unsafe.Slice(&font.glyphs, font.bytePerGlyph*font.numGlyph)
	var x, y, kx, line, mask, offs int
	for i := 0; i < len(s); i++ {
		offs = kx * int(font.width+1) * 4
		for y = 0; y < int(font.height); y++ {
			line = offs
			mask = 1 << 7
			for x = 0; x < int(font.width); x++ {
				var color uint32
				if (int(glyphs[int(s[i])*int(font.height)+y]) & mask) == mask {
					color = 0x00FFFFFF
				}
				*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(line))) = color
				line = line + 4
				mask >>= 1
			}
			*(*uint32)(unsafe.Add(unsafe.Pointer(&fb), uintptr(line))) = 0
			offs = offs + (int)(bootboot.FbScanline)
		}
		kx = kx + 1
	}
}

// All go programs expect there to be a main function even though this is never called
func main() {}
