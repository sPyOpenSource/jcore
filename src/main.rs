#![no_std]
#![no_main]

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
