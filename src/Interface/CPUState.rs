#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]

#[no_mangle]
pub static mut cpuStateMethods: [libc::c_int; 0] = [];

#[no_mangle]
pub static mut stackMethods: [libc::c_int; 0] = [];

#[no_mangle]
pub unsafe extern "C" fn init_cpustate_portal() {}
