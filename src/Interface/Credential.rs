#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
#[no_mangle]
pub static mut credentialvariableClass: *mut libc::c_int = 0 as *const libc::c_int
    as *mut libc::c_int;
#[no_mangle]
pub unsafe extern "C" fn credentialvariable_get(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn credentialvariable_getSignerDomainID(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub static mut credentialvariableMethods: [libc::c_int; 0] = [];
#[no_mangle]
pub unsafe extern "C" fn init_credential_portal() {}
