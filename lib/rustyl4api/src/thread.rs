use crate::utils::MASK;
use core::arch::asm;

fn tpidrro_el0() -> usize {
    let tpidrro: usize;

    unsafe {
        asm!("mrs {0}, tpidrro_el0", out(reg) tpidrro);
    }

    tpidrro
}

pub fn thread_id() -> usize {
    tpidrro_el0() & MASK!(48)
}

pub fn cpu_id() -> usize {
    tpidrro_el0() >> 48
}
