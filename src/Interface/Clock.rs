#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(asm, extern_types, register_tool)]
use std::arch::asm;
extern "C" {
    pub type timeval;
}
#[no_mangle]
pub unsafe extern "C" fn clock_getTicks(mut self_0: *mut libc::c_int) -> libc::c_int {
    let mut ret: libc::c_ulonglong = 0;
    asm!("rdtsc", lateout(A) ret, options(preserves_flags));
    return ret as libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn clock_getTicks_low(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    let mut ret: libc::c_ulonglong = 0;
    asm!("rdtsc", lateout(A) ret, options(preserves_flags));
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn clock_getTicks_high(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    let mut ret: libc::c_ulonglong = 0;
    asm!("rdtsc", lateout(A) ret, options(preserves_flags));
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn clock_getCycles(
    mut self_0: *mut libc::c_int,
    mut cycleTime: *mut libc::c_int,
) -> libc::c_int {
    let mut low: libc::c_ulong = 0;
    let mut high: libc::c_ulong = 0;
    asm!("rdtsc", lateout("dx") high, lateout("ax") low, options(preserves_flags));
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn clock_toMicroSec(
    mut self_0: *mut libc::c_int,
    mut a: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn clock_toNanoSec(
    mut self_0: *mut libc::c_int,
    mut a: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn clock_toMilliSec(
    mut self_0: *mut libc::c_int,
    mut a: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub static mut clockMethods: [libc::c_int; 0] = [];
#[no_mangle]
pub unsafe extern "C" fn init_clock_portal() {}
