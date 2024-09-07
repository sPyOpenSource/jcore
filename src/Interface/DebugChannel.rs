#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
#[no_mangle]
pub unsafe extern "C" fn debug_read(mut self_0: *mut libc::c_int) -> libc::c_int {
    return 'x' as i32;
}
#[no_mangle]
pub unsafe extern "C" fn debug_writeBuf(
    mut self_0: *mut libc::c_int,
    mut arr: *mut libc::c_int,
    mut off: libc::c_int,
    mut len: libc::c_int,
) {}
#[no_mangle]
pub static mut debugChannelMethods: [libc::c_int; 0] = [];
#[no_mangle]
pub unsafe extern "C" fn init_debugchannel_portal() {}
