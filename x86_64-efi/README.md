BOOTBOOT UEFI Implementation
============================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

On [UEFI machines](http://www.uefi.org/), the PCI Option ROM is created from the standard EFI
OS loader application.

Machine state
-------------

IRQs masked. GDT unspecified, but valid, IDT unset. Code is running in supervisor mode in ring 0.

File system drivers
-------------------

For boot partition, UEFI version relies on any file system that's supported by EFI Simple File System Protocol.
This implementation supports both SHA-XOR-CBC and AES-256-CBC cipher.

Installation
------------

1. *UEFI disk*: copy __bootboot.efi__ to **_FS0:\EFI\BOOT\BOOTX64.EFI_**.

2. *UEFI ROM*: use __bootboot.rom__ which is a standard **_PCI Option ROM image_**.

3. *GRUB*, *UEFI Boot Manager*: add __bootboot.efi__ to boot options.

Limitations
-----------

Known limitations:

 - Maps the first 16G of RAM.
 - PCI Option ROM should be signed in order to work.
 - Compressed initrd in ROM is limited to 16M.
