/// mykernel/zig/src/main.zig
///
/// Copyright (C) 2022 binarycraft
///
/// Permission is hereby granted, free of charge, to any person
/// obtaining a copy of this software and associated documentation
/// files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy,
/// modify, merge, publish, distribute, sublicense, and/or sell copies
/// of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be
/// included in all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
/// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
///
/// This file is part of the BOOTBOOT Protocol package.
/// @brief A sample BOOTBOOT compatible kernel
const std = @import("std");
const BOOTBOOT = @import("bootboot.zig").BOOTBOOT;

const fontEmbedded = @embedFile("font.psf");

// imported virtual addresses, see linker script
extern var bootboot: BOOTBOOT; // see bootboot.zig
extern var environment: [4096]u8; // configuration, UTF-8 text key=value pairs
extern var fb: u8; // linear framebuffer mapped

// Display text on screen
const PsfFont = packed struct {
    magic: u32, // magic bytes to identify PSF
    version: u32, // zero
    headersize: u32, // offset of bitmaps in file, 32
    flags: u32, // 0 if there's no unicode table
    numglyph: u32, // number of glyphs
    bytesperglyph: u32, // size of each glyph
    height: u32, // height in pixels
    width: u32, // width in pixels
};

// function to display a string
pub fn puts(string: []const u8) void {
    const font = @bitCast(PsfFont, fontEmbedded[0..@sizeOf(PsfFont)].*);
    var bytesperline = (font.width + 7) / 8;
    var framebuffer = @intToPtr([*]u32, @ptrToInt(&fb));
    for (string) |char, i| {
        var offs = i * (font.width + 1) * 4;
        var idx = if (char > 0 and char < font.numglyph) blk: {
            break :blk font.headersize + (char * font.bytesperglyph);
        } else blk: {
            break :blk font.headersize + (0 * font.bytesperglyph);
        };

        {
            var y: usize = 0;
            while (y < font.height) : (y += 1) {
                var line = offs;
                var mask = @as(u32, 1) << @intCast(u5, font.width - 1);

                {
                    var x: usize = 0;
                    while (x < font.width) : (x += 1) {
                        if ((fontEmbedded[idx] & mask) == 0) {
                            framebuffer[line / @sizeOf(u32)] = 0x000000;
                        } else {
                            framebuffer[line / @sizeOf(u32)] = 0xFFFFFF;
                        }
                        mask >>= 1;
                        line += 4;
                    }
                }

                framebuffer[line / @sizeOf(u32)] = 0;
                idx += bytesperline;
                offs += bootboot.fb_scanline;
            }
        }
    }
}

// Entry point, called by BOOTBOOT Loader
export fn _start() callconv(.Naked) noreturn {
    // NOTE: this code runs on all cores in parallel
    var s = bootboot.fb_scanline;
    var w = bootboot.fb_width;
    var h = bootboot.fb_height;
    var framebuffer = @intToPtr([*]u32, @ptrToInt(&fb));

    if (s > 0) {
        // cross-hair to see screen dimension detected correctly
        {
            var y: usize = 0;
            while (y < h) : (y += 1) {
                framebuffer[(s * y + w * 2) / @sizeOf(u32)] = 0x00FFFFFF;
            }
        }

        {
            var x: usize = 0;
            while (x < w) : (x += 1) {
                framebuffer[(s * (h / 2) + x * 4) / @sizeOf(u32)] = 0x00FFFFFF;
            }
        }

        // red, green, blue boxes in order
        {
            var y: usize = 0;
            while (y < 20) : (y += 1) {
                var x: usize = 0;
                while (x < 20) : (x += 1) {
                    framebuffer[(s * (y + 20) + (x + 20) * 4) / @sizeOf(u32)] = 0x00FF0000;
                }
            }
        }

        {
            var y: usize = 0;
            while (y < 20) : (y += 1) {
                var x: usize = 0;
                while (x < 20) : (x += 1) {
                    framebuffer[(s * (y + 20) + (x + 50) * 4) / @sizeOf(u32)] = 0x0000FF00;
                }
            }
        }

        {
            var y: usize = 0;
            while (y < 20) : (y += 1) {
                var x: usize = 0;
                while (x < 20) : (x += 1) {
                    framebuffer[(s * (y + 20) + (x + 80) * 4) / @sizeOf(u32)] = 0x000000FF;
                }
            }
        }

        // say hello
        puts("Hello from a simple BOOTBOOT kernel");
    }
    // hang for now
    while (true) {}
}
