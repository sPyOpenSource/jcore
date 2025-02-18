#![no_std]
#![no_main]
//#![feature(uefi_std)]

mod vga_buffer;

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

//use std::os::uefi as uefi_std;
//use uefi::runtime::ResetType;
use uefi::{Handle, Status};

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

#[no_mangle]
pub extern "C" fn _start() -> ! {
    vga_buffer::WRITER.lock().write_str("Hello again").unwrap();
    write!(vga_buffer::WRITER.lock(), ", some numbers: {} {}", 42, 1.337).unwrap();
    loop {}
}

#[no_mangle]
extern fn efi_main() {
    //println!("Hello World from uefi_std");
    //setup_uefi_crate();
    //println!("UEFI-Version is {}", uefi::system::uefi_revision());
    //uefi::runtime::reset(ResetType::SHUTDOWN, Status::SUCCESS, None);
}
