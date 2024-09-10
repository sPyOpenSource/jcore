#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(asm, extern_types, register_tool)]
use std::arch::asm;
extern "C" {
    pub type MemoryProxy_s;
}

#[no_mangle]
pub unsafe extern "C" fn debugsupport_dumpDomain(
    mut self_0: *mut libc::c_int,
    mut domainObj: *mut libc::c_int,
) {}

#[no_mangle]
pub unsafe extern "C" fn debugsupport_dumpObject(
    mut self_0: *mut libc::c_int,
    mut obj: *mut libc::c_int,
) {}

#[no_mangle]
pub unsafe extern "C" fn debugsupport_sendBinary(
    mut self_0: *mut libc::c_int,
    mut name: *mut libc::c_int,
    mut data: *mut MemoryProxy_s,
    mut size: libc::c_int,
) {}

#[no_mangle]
pub unsafe extern "C" fn debugsupport_registerMonitorCommand(
    mut self_0: *mut libc::c_int,
    mut name: *mut libc::c_int,
    mut cmd: *mut libc::c_int,
) {}

#[no_mangle]
pub unsafe extern "C" fn debugsupport_breakpoint(mut self_0: *mut libc::c_int) {
    asm!("int $3", options(preserves_flags, att_syntax));
}

#[no_mangle]
pub static mut debugsupportMethods: [libc::c_int; 0] = [];

#[no_mangle]
pub unsafe extern "C" fn init_debugsupport_portal() {}
