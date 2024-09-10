#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]

#[no_mangle]
pub static mut atomicvariableClass: *mut libc::c_int = 0 as *const libc::c_int
    as *mut libc::c_int;

#[no_mangle]
pub unsafe extern "C" fn atomicvariable_set(
    mut self_0: *mut libc::c_int,
    mut value: *mut libc::c_int,
) {}

#[no_mangle]
pub unsafe extern "C" fn atomicvariable_get(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub static mut dline: libc::c_int = 0 as libc::c_int;

#[no_mangle]
pub unsafe extern "C" fn atomicvariable_activateListMode(mut self_0: *mut libc::c_int) {}

#[no_mangle]
pub static mut atomicvariableMethods: [libc::c_int; 0] = [];

#[no_mangle]
pub unsafe extern "C" fn init_atomicvariable_portal() {}
