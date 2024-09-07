#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
extern "C" {
    fn printf(_: *const libc::c_char, _: ...) -> libc::c_int;
    fn strlen(_: *const libc::c_char) -> libc::c_ulong;
    fn exit(_: libc::c_int) -> !;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_receive(
    mut self_0: *mut libc::c_int,
    mut obj: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_yield(mut self_0: *mut libc::c_int) -> libc::c_int {
    threadyield();
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_sleep(
    mut self_0: *mut libc::c_int,
    mut msec: libc::c_int,
    mut nsec: libc::c_int,
) {
    sys_panic(
        b"SLEEP NO LONGER SUPPORTED. USE jx.timer.SleepManager\0" as *const u8
            as *const libc::c_char,
    );
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_wait(mut source: *mut libc::c_int) -> libc::c_int {
    sys_panic(b"should never be executed\0" as *const u8 as *const libc::c_char);
    return 0 as libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_notify(mut source: *mut libc::c_int) -> libc::c_int {
    sys_panic(b"should never be executed\0" as *const u8 as *const libc::c_char);
    return 0 as libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_notifyAll(
    mut source: *mut libc::c_int,
) -> libc::c_int {
    sys_panic(b"should never be executed\0" as *const u8 as *const libc::c_char);
    return 0 as libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn dumpVTable(mut o: *mut libc::c_int, mut n: libc::c_int) {}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_switchTo(
    mut self_0: *mut libc::c_int,
    mut cpuState: *mut libc::c_int,
) -> libc::c_int {
    sys_panic(b"switchTo should not be called\0" as *const u8 as *const libc::c_char);
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getCPUState(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    return thread2CPUState(curthr()) as *mut libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_block(mut self_0: *mut libc::c_int) {
    threadblock();
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_clearUnblockFlag(mut self_0: *mut libc::c_int) {}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_waitUntilBlocked(
    mut self_0: *mut libc::c_int,
    mut cpuStateProxy: *mut libc::c_int,
) -> libc::c_int {
    sys_panic(
        b"waituntilblocked should not be called\0" as *const u8 as *const libc::c_char,
    );
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_join(
    mut self_0: *mut libc::c_int,
    mut cpuStateProxy: *mut libc::c_int,
) {
    sys_panic(
        b"cpuManager_join should not be called\0" as *const u8 as *const libc::c_char,
    );
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_createCPUState(
    mut self_0: *mut libc::c_int,
    mut entry: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_printStackTrace(mut self_0: *mut libc::c_int) {}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getAtomicVariable(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_attachToThread(
    mut self_0: *mut libc::c_int,
    mut portalParameter: *mut libc::c_int,
) {}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getAttachedObject(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getCredential(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_recordEvent(
    mut self_0: *mut libc::c_int,
    mut nr: libc::c_int,
) {}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_recordEventWithInfo(
    mut self_0: *mut libc::c_int,
    mut nr: libc::c_int,
    mut info: libc::c_int,
) {}
#[no_mangle]
pub unsafe extern "C" fn createNewEvent(mut label: *mut libc::c_char) -> libc::c_int {
    return -(1 as libc::c_int);
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_createNewEvent(
    mut self_0: *mut libc::c_int,
    mut label: *mut libc::c_int,
) -> libc::c_int {
    return -(1 as libc::c_int);
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getClass(
    mut self_0: *mut libc::c_int,
    mut nameObj: *mut libc::c_int,
) -> *mut libc::c_int {
    let mut name: [libc::c_char; 128] = [0; 128];
    let mut i: libc::c_int = 0;
    i = 0 as libc::c_int;
    while (i as libc::c_ulong) < strlen(name.as_mut_ptr()) {
        if name[i as usize] as libc::c_int == '.' as i32 {
            name[i as usize] = '/' as i32 as libc::c_char;
        }
        i += 1;
    }
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getVMClass(
    mut self_0: *mut libc::c_int,
    mut obj: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getVMObject(
    mut self_0: *mut libc::c_int,
) -> *mut libc::c_int {
    return allocVMObjectProxyInDomain(curdom()) as *mut libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_executeClassConstructors(
    mut self_0: *mut libc::c_int,
    mut id: libc::c_int,
) {
    let mut i: libc::c_int = 0;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_inheritServiceThread(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    return 0 as libc::c_int;
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_reboot(mut self_0: *mut libc::c_int) {
    exit(0 as libc::c_int);
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_getStackDepth(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_inhibitScheduling(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub unsafe extern "C" fn cpuManager_allowScheduling(
    mut self_0: *mut libc::c_int,
) -> libc::c_int {
    panic!("Reached end of non-void function without returning");
}
#[no_mangle]
pub static mut cpuManagerMethods: [libc::c_int; 0] = [];
#[no_mangle]
pub unsafe extern "C" fn init_cpumanager_portal() {}
