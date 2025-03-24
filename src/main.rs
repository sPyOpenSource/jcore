#![no_std]
#![no_main]
//#![feature(uefi_std)]

//mod vga_buffer;
mod serial;
//extern crate alloc;

/*mod boot {
    use core::arch::global_asm;

    global_asm!{
        "*.section .text._start"
    }
}*/

/*fn main() {
    let stdout = stdout();
    let message = String::from("Hello fellow Rustaceans!");
    println!("{message}");
    let width = message.chars().count();

    let mut writer = BufWriter::new(stdout.lock());
    say(message.as_bytes(), width, &mut writer).unwrap();
}*/

//use std::io::{stdout, BufWriter};
use core::panic::PanicInfo;
use core::fmt::Write;
//use alloc::vec;
//use alloc::vec::Vec;
//use std::os::uefi as uefi_std;
use uefi::runtime::ResetType;
use uefi::{Handle, Status};
use uefi::prelude::*;
use uefi::proto::console::gop::GraphicsOutput;
use uefi::proto::console::gop::BltPixel;
use uefi::proto::console::gop::BltOp;
use uefi::proto::console::gop::BltRegion;
use uefi::proto::rng::Rng;
use uefi::proto::loaded_image::LoadedImage;
use uefi::boot;
use log::info;

// Performs the necessary setup code for the `uefi` crate.
/*fn setup_uefi_crate() {
    let st = uefi_std::env::system_table();
    let ih = uefi_std::env::image_handle();

    // Mandatory setup code for `uefi` crate.
    unsafe {
        uefi::table::set_system_table(st.as_ptr().cast());

        let ih = Handle::from_ptr(ih.as_ptr().cast()).unwrap();
        uefi::boot::set_image_handle(ih);
    }
}*/

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[entry]
fn main() -> Status {
    //vga_buffer::WRITER.lock().write_str("Hello again").unwrap();
    //write!(vga_buffer::WRITER.lock(), ", some numbers: {} {}", 42, 1.337).unwrap();

    serial_println!("Hello World");
    uefi::helpers::init().unwrap();
    //let stdout = st.stdout();

    //writeln!(stdout, "Hello, UEFI in Rust!").unwrap();
    // Open graphics output protocol.
    //let gop_handle = Boot::get_handle_for_protocol::<GraphicsOutput>()?;
    //let mut gop = Boot::open_protocol_exclusive::<GraphicsOutput>(gop_handle)?;

    // Open random number generator protocol.
    //let rng_handle = Boot::get_handle_for_protocol::<Rng>()?;
    //let mut rng = Boot::open_protocol_exclusive::<Rng>(rng_handle)?;

    // Create a buffer to draw into.
    //let (width, height) = gop.current_mode_info().resolution();
    //let mut buffer = Buffer::new(width, height);
    //setup_uefi_crate();
let loaded_image = boot::open_protocol_exclusive::<LoadedImage>(boot::image_handle());

    loop {};
    Status::SUCCESS
    //println!("UEFI-Version is {}", uefi::system::uefi_revision());
    //uefi::runtime::reset(ResetType::SHUTDOWN, Status::SUCCESS, None);
}

struct Buffer {
    width: usize,
    height: usize,
    //pixels: [BltPixel],
}

impl Buffer {
    /// Create a new `Buffer`.
    fn new(width: usize, height: usize) -> Self {
        Buffer {
            width,
            height,
            //pixels: [BltPixel; width * height],
        }
    }

    // Get a single pixel.
    /*fn pixel(&mut self, x: usize, y: usize) -> Option<&mut BltPixel> {
        //self.pixels[y * self.width + x]
    }*/

    // Blit the buffer to the framebuffer.
    /*fn blit(&self, gop: &mut GraphicsOutput) -> Result {
        gop.blt(BltOp::BufferToVideo {
            buffer: &self.pixels,
            src: BltRegion::Full,
            dest: (0, 0),
            dims: (self.width, self.height),
        })
    }*/

    // Update only a pixel to the framebuffer.
    /*fn blit_pixel(
        &self,
        gop: &mut GraphicsOutput,
        coords: (usize, usize),
    ) -> Result {
        gop.blt(BltOp::BufferToVideo {
            buffer: &self.pixels,
            src: BltRegion::SubRectangle {
                coords,
                px_stride: self.width,
            },
            dest: coords,
            dims: (1, 1),
        })
    }*/
}
