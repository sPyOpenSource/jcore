#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
extern "C" {
    fn printf(_: *const libc::c_char, _: ...) -> libc::c_int;
}

#[no_mangle]
pub static mut dddd: libc::c_int = 9 as libc::c_int;

#[no_mangle]
pub static mut linenumber: libc::c_int = 0 as libc::c_int;

#[no_mangle]
pub unsafe extern "C" fn domainManager_createDomain(
    mut self_0: *mut libc::c_int,
    mut dname: *mut libc::c_int,
    mut cpuObjs: *mut libc::c_int,
    mut HLSNames: *mut libc::c_int,
    mut dcodeName: *mut libc::c_int,
    mut libsName: *mut libc::c_int,
    mut startClassName: *mut libc::c_int,
    mut gcinfo0: libc::c_int,
    mut gcinfo1: libc::c_int,
    mut gcinfo2: libc::c_int,
    mut gcinfo3: *mut libc::c_int,
    mut gcinfo4: libc::c_int,
    mut codeSize: libc::c_int,
    mut argv: *mut libc::c_int,
    mut naming: *mut libc::c_int,
    mut portals: *mut libc::c_int,
    mut gcImpl: libc::c_int,
    mut schedInfo: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn domainManager_getDomainZero(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    printf(b"getDomainZero \n\0" as *const u8 as *const libc::c_char);
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn domainManager_getCurrentDomain(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn findZeroLibMethodIndex(
    mut domain: *mut libc::c_int,
    mut className: *mut libc::c_char,
    mut methodName: *mut libc::c_char,
    mut signature: *mut libc::c_char,
) -> libc::c_int {
    sys_panic(
        b"Cannot find DEP method %s:: %s%s\n\0" as *const u8 as *const libc::c_char,
        className,
        methodName,
        signature,
    );
    return 0 as libc::c_int;
}

#[no_mangle]
pub unsafe extern "C" fn domainManager_installInterceptor(
    mut self_0: *mut libc::c_int,
    mut domainObj: *mut libc::c_int,
    mut interceptor: *mut libc::c_int,
    mut interceptorThread: *mut libc::c_int,
) {
    exceptionHandler(-(1 as libc::c_int));
}

#[no_mangle]
pub unsafe extern "C" fn installInterceptor(
    mut self_0: *mut libc::c_int,
    mut domain: *mut libc::c_int,
    mut interceptor: *mut libc::c_int,
    mut thread: *mut libc::c_int,
) {
    sys_panic(b"\0" as *const u8 as *const libc::c_char);
}

#[no_mangle]
pub unsafe extern "C" fn domainManager_terminateCaller(mut self_0: *mut libc::c_int) {}

#[no_mangle]
pub unsafe extern "C" fn domainManager_freeze(
    mut self_0: *mut libc::c_int,
    mut domainObj: *mut libc::c_int,
) {}

#[no_mangle]
pub unsafe extern "C" fn domainManager_thaw(
    mut self_0: *mut libc::c_int,
    mut domainObj: *mut libc::c_int,
) {}

#[no_mangle]
pub static mut domainManagerMethods: [libc::c_int; 0] = [];

#[no_mangle]
pub unsafe extern "C" fn init_domainmanager_portal() {}
