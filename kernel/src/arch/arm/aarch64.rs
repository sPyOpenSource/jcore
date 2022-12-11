use core::arch::asm;
use core::convert::TryInto;

pub fn mpidr_el1() -> u64 {
    let x;
    unsafe {
        asm!("mrs {0}, mpidr_el1", out(reg) x);
    }
    x
}

pub fn cpuid() -> usize {
    (mpidr_el1() as usize) & !0xc1000000
}

pub fn isb() {
    unsafe { asm!("isb") }
}

#[inline(always)]
pub fn dsb() {
    unsafe { asm!("dsb sy") }
}

#[inline(always)]
pub fn dmb() {
    unsafe {
        asm!("dmb sy");
    }
}

#[inline(always)]
pub fn get_elr() -> usize {
    let elr;
    unsafe {
        asm!("mrs {0}, elr_el1", out(reg) elr);
    }
    elr
}

#[inline(always)]
pub fn get_esr() -> u32 {
    let esr:u64;
    unsafe {
        asm!("mrs {0}, esr_el1", out(reg) esr);
    }
    esr.try_into().unwrap()
//    esr
}

#[inline(always)]
pub fn get_far() -> u64 {
    let far;
    unsafe {
        asm!("mrs {0}, far_el1", out(reg) far);
    }
    far
}

pub fn wfe() {
    unsafe {
        asm!("wfe");
    }
}

pub fn wfi() {
    unsafe {
        asm!("wfi");
    }
}

pub fn dc_clean_by_va_PoU(vaddr: usize) {
    unsafe {
        asm!("dc cvau, {0}", in(reg) vaddr);
    }
    dsb();
}

pub fn clid() -> usize {
    let clid: usize;
    unsafe {
        asm!("mrs {0}, clidr_el1", out(reg) clid);
    }

    return clid;
}

pub fn cache_type(clid: usize, level: usize) -> usize {
    (clid >> (level * 3)) & MASK!(3)
}

pub fn read_cache_size(level: usize, instruction: bool) -> usize {
    let cssr_old: usize;
    let size: usize;
    let selector: usize;

    if level < 1 { selector = instruction as usize }
            else { selector = (instruction as usize) | 1 };

    unsafe {
        asm!("mrs {0}, csselr_el1",
             "msr csselr_el1, {1}",
             out(reg) cssr_old, in(reg) selector);
        asm!("mrs {0}, ccsidr_el1",
             "msr csselr_el1, {1}",
             out(reg) size, in(reg) cssr_old);
    }

    size
}

pub fn clean_dcache_poc() {
    let clid = clid();
    let loc = (clid >> 24) & MASK!(3); // level of coherence

    for l in 0..loc {
        if cache_type(clid, l) > 0b001 {
            // ICache
            let s = read_cache_size(l, false);
            let line_bits = (s & MASK!(3)) + 4;
            let assoc = ((s >> 3) & MASK!(10)) + 1;
            let assoc_bits = 64 - (assoc - 1).leading_zeros();
            let nsets = ((s >> 13) & MASK!(15)) + 1;

            for w in 0..assoc {
                for s in 0..nsets {
                    let wsl = w << (32 - assoc_bits) | (s << line_bits) | (l << 1);
                    unsafe { asm!("dc cisw, {0}", in(reg) wsl) }
                }
            }
        }
    }
}

pub fn clean_l1_cache() {
    dsb();
    clean_dcache_poc();
    dsb();
    unsafe { asm!("ic iallu") }
    isb();
    dsb();
}
