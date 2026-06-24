.PHONY: all baremetal original qemu qemu8 qemu4 clean

all: baremetal

baremetal:
	$(MAKE) -C baremetal

original:
	$(MAKE) -C original

qemu:
	$(MAKE) -C baremetal qemu

qemu8:
	$(MAKE) -C baremetal qemu8

qemu4:
	$(MAKE) -C baremetal qemu8-raspi4

clean:
	$(MAKE) -C baremetal clean
	$(MAKE) -C original clean
