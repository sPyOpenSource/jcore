# RPi Boot Chain Resources

## Knowledge

- [Raspberry Pi Official Documentation: Boot Flow (EEPROM)](https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/raspberry-pi/bootflow-eeprom.adoc)
  Official RPi 4+ EEPROM boot flow. Use for: understanding second-stage SPI bootloader, BOOT_ORDER config.
- [Raspberry Pi Official Documentation: Boot Flow (Legacy)](https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/raspberry-pi/bootflow-legacy.adoc)
  Legacy BCM2837 boot flow (RPi 3 and earlier). Use for: understanding bootcode.bin and start.elf.
- [Raspberry Pi Official: Boot Modes](https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/raspberry-pi/bootmodes.adoc)
  SD, USB, Network boot modes. Use for: USB host boot, TFTP network boot setup.
- [Raspberry Pi Official: config.txt Boot Options](https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/config_txt/boot.adoc)
  Reference for kernel, kernel_address, arm_64bit, device_tree, etc. Use for: configuring your kernel load.
- [Raspberry Pi Official: Boot Folder Contents](https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/configuration/reference.adoc)
  What each file in the boot partition does. Use for: SD card boot partition layout.
- [rpi4-osdev.com — Part 1: Bootstrapping](https://www.rpi4os.com/part1-bootstrapping/)
  Practical tutorial series. Use for: step-by-step bare-metal AArch64 RPi4 kernel.
- [OSDev Wiki: Raspberry Pi Bare Bones](https://wiki.osdev.org/Raspberry_Pi_Bare_Bones)
  Community wiki covering entry point, linker script, MMIO base addresses for all models. Use for: cross-referencing boot conventions.
- [GitHub: s-matyukevich/raspberry-pi-os](https://github.com/s-matyukevich/raspberry-pi-os)
  Open-source OS tutorial with lesson breakdown. Use for: seeing AArch64 kernel structure and drivers.
- [GitHub: rhythm16/rpi4-bare-metal](https://github.com/rhythm16/rpi4-bare-metal)
  Bare-metal RPi4 with UART, GIC, scheduler, virtual memory. Use for: practical reference implementation.
- [Raspberry Pi 4 Boot Security White Paper](https://pip-assets.raspberrypi.com/categories/1260-security/documents/RP-004651-WP-2-Raspberry%20Pi%204%20Boot%20Security.pdf)
  Official RPi Ltd paper on secure boot chain. Use for: understanding signed boot and OTP.
- [Baking Pi — Cambridge University Course](https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/)
  Original 32-bit bare-metal course. Use for: conceptual grounding in MMIO, GPIO, interrupt handling.

## Wisdom (Communities)

- [r/osdev on Reddit](https://reddit.com/r/osdev)
  Active community for OS development discussion. Use for: debugging boot problems on real hardware.
- [OSDev.org Forum](https://forum.osdev.org/)
  Established forum with deep archives on ARM64 boot. Use for: linker script help, boot debugging.
- [Raspberry Pi Forums — Bare-metal](https://forums.raspberrypi.com/viewforum.php?f=72)
  First-party RPi bare-metal forum. Use for: firmware-specific questions, config.txt quirks.
