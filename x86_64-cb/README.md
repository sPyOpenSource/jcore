BOOTBOOT Coreboot x86_64 Implementation
=======================================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

Implements the BOOTBOOT Protocol as a [coreboot](https://coreboot.org) payload. Currently __EXPERIMENTAL__.
Must be compiled using the coreboot build environment.

Compilation
-----------

### Step 1 - Install dependencies

First, install the [coreboot dependencies](https://doc.coreboot.org/tutorial/part1.html): bison, build-essentials, curl,
flex, git, gnat, libncurses5-dev, m4, zlib. Please refer to the linked doc for up-to-date list of dependencies.

### Step 2 - Get coreboot

Download coreboot and its submodules
```sh
$ git clone https://review.coreboot.org/coreboot
$ cd coreboot
$ git submodule update --init
```
It is very important to initialize submodules, otherwise you won't be able to compile coreboot.

### Step 3 - Create toolchain

This step could take a while. Replace *$(nproc)* with the number of CPU cores you have.
```sh
$ make crossgcc-i386 CPUS=$(nproc)
```

### Step 4 - Patch coreboot and configure

Copy and overwrite files from [../coreboot directory](https://gitlab.com/bztsrc/bootboot/tree/master/coreboot) to the
coreboot working directory. This will update the coreboot configuration system.

Now configure coreboot for your motherboard (or qemu) and BOOTBOOT.
```sh
$ make menuconfig
    select 'Mainboard' menu
    Beside 'Mainboard vendor' should be '(Emulation)'
    Beside 'Mainboard model' should be 'QEMU x86 i440fx/piix4'
    select 'Exit'
    select 'Payload' menu
    select 'Add a Payload'
    choose 'BOOTBOOT'
    select 'Exit'
    select 'Exit'
    select 'Yes'
```

### Step 5 - Build coreboot

```sh
$ make
```

### Step 6 - Test the newly compiled ROM in QEMU

For more information, read [coreboot docs](https://doc.coreboot.org/mainboard/emulation/qemu-i440fx.html).
```sh
$ qemu-system-x86_64 -bios $(COREBOOT)/build/coreboot.rom -drive file=$(BOOTBOOT)/images/disk-x86.img,format=raw -serial stdio
```

Machine state
-------------

IRQs masked. GDT unspecified, but valid, IDT unset. SSE, SMP enabled. Code is running in supervisor mode in ring 0 on all cores.

Installation
------------

Once you have compiled a coreboot ROM with the BOOTBOOT payload, you can
[flash the build/coreboot.rom](https://doc.coreboot.org/flash_tutorial/index.html) file to your mainboard.

Limitations
-----------

 - As it boots in protected mode, it only maps the first 4G of RAM.
 - The CMOS nvram does not store timezone, so always GMT+0 returned in bootboot.timezone.
 - Only supports SHA-XOR-CBC, no AES
