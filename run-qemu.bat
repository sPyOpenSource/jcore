qemu-system-aarch64 -bios RELEASEAARCH64_QEMU_EFI.fd -drive format=raw,unit=0,file=drive/drive.hdd -m 4096m -M virt -cpu max -device virtio-gpu-pci -display sdl -boot menu=on,splash-time=0 -usb -device nec-usb-xhci,id=xhci -device usb-kbd

pause
