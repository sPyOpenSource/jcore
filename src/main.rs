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

static HELLO: &[u8] = b"Hello World!";

//3F20_0008 fsel2 1<<3 turn pin21 into an output
//3F20_001c gpio1_set 1<<21 pin 21 on
//3F20_0028 gpio1_clear 1<<21 turns pin 21 off

#[no_mangle]
pub extern "C" fn main() -> ! {
    /*let vga_buffer = 0xb8000 as *mut u8;

    for (i, &byte) in HELLO.iter().enumerate() {
        unsafe {
            *vga_buffer.offset(i as isize * 2) = byte;
            *vga_buffer.offset(i as isize * 2 + 1) = 0xb;
        }
    }
    vga_buffer::print_something();*/
    /*unsafe {
        // turn PIN21 into an output
        core::ptr::write_volatile(0x3F20_0008 as *mut u32, 1<<3);

        loop {
            // turn pin on
            core::ptr::write_volatile(0x3f20001c as *mul u32, 1<<21);
            for _ in 1..50000 {
                asm!("nop");
            }

            // turn pin off
            core::ptr::write_volatile(0x3f200028 as *mul u32, 1<<21);
            for _ in 1..50000 {
                asm!("nop");
            }
        }
    }*/
    vga_buffer::WRITER.lock().write_str("Hello again").unwrap();
    write!(vga_buffer::WRITER.lock(), ", some numbers: {} {}", 42, 1.337).unwrap();
    loop {}
}
