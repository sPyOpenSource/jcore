#qemu-system-aarch64 -bios RELEASEAARCH64_QEMU_EFI.fd -drive format=raw,unit=0,file=/Users/xuyi/Downloads/OVMF-X64-r15214/test.img -m 4096m -M virt -cpu max -device virtio-gpu-pci -boot menu=on,splash-time=0 -usb -device nec-usb-xhci,id=xhci -device usb-kbd
qemu-system-aarch64 -kernel kernel8.img -M raspi3b
