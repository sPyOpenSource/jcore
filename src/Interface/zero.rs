#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case, non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
extern "C" {
    fn IN_jxbytecpy(
        source: *mut libc::c_char,
        target: *mut libc::c_char,
        nbytes: Option::<jint>,
    );
    fn printf(_: *const libc::c_char, _: ...) -> libc::c_int;
}
pub type jint = unsafe extern "C" fn(*mut libc::c_int) -> libc::c_int;

#[no_mangle]
pub unsafe extern "C" fn getInitialNaming() -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn installInitialNaming(
    mut srcDomain: *mut libc::c_int,
    mut dstDomain: *mut libc::c_int,
    mut naming: *mut libc::c_int,
) {
    thread_prepare_to_copy();
}

#[no_mangle]
pub unsafe extern "C" fn receive_dep(mut arg: *mut libc::c_void) {}

#[no_mangle]
pub unsafe extern "C" fn createClassDescImplementingInterface(
    mut domain: *mut libc::c_int,
    mut cl: *mut libc::c_int,
    mut methods: *mut libc::c_int,
    mut numMethods: libc::c_int,
    mut name: *mut libc::c_char,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn createSubClass(
    mut cl: *mut libc::c_int,
    mut classDesc: *mut libc::c_int,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn init_zero_class(
    mut ifname: *mut libc::c_char,
    mut methods: *mut libc::c_int,
    mut size: Option::<jint>,
    mut instanceSize: Option::<jint>,
    mut typeMap: *mut libc::c_int,
    mut subname: *mut libc::c_char,
) -> *mut libc::c_int {
    extern "C" {
        #[link_name = "mapBytes"]
        fn mapBytes_0(_: *mut libc::c_int) -> libc::c_int;
    }
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn init_zero_dep(
    mut ifname: *mut libc::c_char,
    mut depname: *mut libc::c_char,
    mut methods: *mut libc::c_int,
    mut size: Option::<jint>,
    mut subname: *mut libc::c_char,
) -> *mut libc::c_int {
    CHECK_STACK_SIZE(ifname, 256 as libc::c_int);
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn init_zero_dep_without_thread(
    mut ifname: *mut libc::c_char,
    mut depname: *mut libc::c_char,
    mut methods: *mut libc::c_int,
    mut size: Option::<jint>,
    mut subname: *mut libc::c_char,
) -> *mut libc::c_int {
    panic!("Reached end of non-void function without returning");
}

#[no_mangle]
pub unsafe extern "C" fn jxbytecpy(
    mut source: *mut libc::c_char,
    mut target: *mut libc::c_char,
    mut nbytes: Option::<jint>,
) {
    IN_jxbytecpy(source, target, nbytes);
}

#[no_mangle]
pub unsafe extern "C" fn addZeroVtables() {}
unsafe extern "C" fn create_CPUObjs() {
    extern "C" {
        static mut cpuClass: *mut libc::c_int;
    }
    let mut cpu_ID: libc::c_int = 0;
    cpu_ID = 0 as libc::c_int;
}

#[no_mangle]
pub static mut monitorThread: *mut libc::c_int = 0 as *const libc::c_int
    as *mut libc::c_int;

#[no_mangle]
pub unsafe extern "C" fn init_zero_from_lib(
    mut domain: *mut libc::c_int,
    mut zeroLib: *mut libc::c_int,
) {
    init_atomicvariable_portal();
    init_bootfs_portal();
    init_cas_portal();
    init_clock_portal();
    init_componentmanager_portal();
    init_cpu_portal();
    init_cpumanager_portal();
    init_cpustate_portal();
    init_credential_portal();
    init_debugchannel_portal();
    init_debugsupport_portal();
    init_domain_portal();
    init_domainmanager_portal();
    init_debugsupport_portal();
    init_irq_portal();
    init_memory_portal();
    init_memorymanager_portal();
    init_mutex_portal();
    init_naming_portal();
    init_ports_portal();
    init_profiler_portal();
    init_scheduler_portal();
    init_smpcpumanager_portal();
    init_vmclass_portal();
    init_vmmethod_portal();
    init_vmobject_portal();
    init_interceptInboundInfo_portal();
    addZeroVtables();
    init_object();
}
