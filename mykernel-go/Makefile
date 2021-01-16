#
# mykernel-go/Makefile
#
# Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# This file is part of the BOOTBOOT Protocol package.
# @brief An example Makefile for sample kernel
#
#

GOFLAGS = -Wall -fpic -fno-stack-protector -static -nostdinc -nostdlib -nostartfiles -nodefaultlibs

all: mykernel.x86_64.elf mykernel.aarch64.elf

mykernel.x86_64.elf: kernel.go
	x86_64-elf-gccgo $(GOFLAGS) -c kernel.go -o kernel.o
	x86_64-elf-ld -nostdlib -nostartfiles -T link.ld kernel.o -o mykernel.x86_64.elf
	#x86_64-elf-strip -s -K mmio -K fb -K bootboot -K environment mykernel.x86_64.elf
	x86_64-elf-readelf -hls mykernel.x86_64.elf >mykernel.x86_64.txt

mykernel.aarch64.elf: kernel.go
	aarch64-elf-gccgo $(GOFLAGS) -c kernel.go -o kernel.o
	aarch64-elf-ld -nostdlib -nostartfiles -T link.ld kernel.o -o mykernel.aarch64.elf
	aarch64-elf-strip -s -K mmio -K fb -K bootboot -K environment mykernel.aarch64.elf
	aarch64-elf-readelf -hls mykernel.aarch64.elf >mykernel.aarch64.txt

clean:
	rm *.o *.elf *.txt || true
