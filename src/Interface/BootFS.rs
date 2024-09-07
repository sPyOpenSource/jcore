#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
#[no_mangle]
pub unsafe extern "C" fn bootfs_lookup(
    mut self_0: *mut libc::c_int,
    mut filename: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub static mut bootfsMethods: [libc::c_int; 0] = [];
#[no_mangle]
pub unsafe extern "C" fn init_bootfs_portal() {}
