BOOTBOOT Coreboot x86_64 Implementáció
======================================

Általános leírásért lásd a [BOOTBOOT Protokoll](https://gitlab.com/bztsrc/bootboot)t.

Ez [coreboot](https://coreboot.org) payloadként implementálja a BOOTBOOT Protokollt. Jelenleg __FEJLESZTÉS ALATT__.
A coreboot fordítási környezetben fordítandó.

Fordítás
--------

### 1. lépés - Függőségek telepítése

Először is, telepíteni kell a [coreboot függőségei](https://doc.coreboot.org/tutorial/part1.html)t: bison, build-essentials, curl,
flex, git, gnat, libncurses5-dev, m4, zlib. A legfrissebb listáért nézd meg a linkelt dokumentumot.

### 2. lépés - A coreboot beszerzése

Töltsd le a coreboot-ot minden almoduljával együtt
```sh
$ git clone https://review.coreboot.org/coreboot
$ cd coreboot
$ git submodule update --init
```
Nagyon fontos, hogy inicializáld az almodulokat, máskülönben nem fog lefordulni a coreboot.

### 3. lépés - Fordítókörnyezet (toolchain) létrehozása

Ez el fog tartani egy jódarabig. A *$(nproc)* helyett add meg, hogy hány CPU core-od van.
```sh
$ make crossgcc-i386 CPUS=$(nproc)
```

### 4. lépés - A coreboot pecselése és konfigurálása

Másold át és írd felül a [../coreboot könyvtár](https://gitlab.com/bztsrc/bootboot/tree/master/coreboot) fájlait a
coreboot projekted munkakönyvtárába. Ez frissíteni fogja a coreboot konfigurációs rendszerét.

Ezután be kell konfigurálnod a coreboot-ot, hogy milyen alaplapot (vagy qemu-t) kezeljen, és hogy BOOTBOOT-al induljon.
```sh
$ make menuconfig
    válaszd a 'Mainboard' menüt
    alatta a 'Mainboard vendor' legyen '(Emulation)'
    alatta a 'Mainboard model' legyen 'QEMU x86 i440fx/piix4'
    válaszd az 'Exit'-t
    válaszd a 'Payload' menüt
    válaszd az 'Add a Payload' opciót
    válaszd ki a 'BOOTBOOT'-ot
    válaszd az 'Exit'-t
    válaszd az 'Exit'-t
    válaszd a 'Yes'-t
```

### 5. lépés - A coreboot fordítása

```sh
$ make
```

### 6. lépés - A frissen fordított ROM tesztelése QEMU-n

Bővebb információért lásd a [coreboot dokumentáció](https://doc.coreboot.org/mainboard/emulation/qemu-i440fx.html)t.
```sh
$ qemu-system-x86_64 -bios $(COREBOOT)/build/coreboot.rom -drive file=$(BOOTBOOT)/images/disk-x86.img,format=raw -serial stdio
```

Gép állapot
-----------

IRQ-k letiltva, GDT nincs meghatározva, de érvényes, IDT nincs beállítva. SSE, SMP engedélyezve. Kód felügyeleti módban, 0-ás gyűrűn
fut minden processzormagon.

Telepítés
---------

Miután lefordítottad a coreboot ROM-ot BOOTBOOT payloaddal, [flashelheted a build/coreboot.rom](https://doc.coreboot.org/flash_tutorial/index.html)
fájlt az alaplapodra.

Limitációk
----------

 - Mivel védett módban indul, csak az első 4G-nyi RAM-ot képezi le.
 - A CMOS nvram nem tárol időzónát, ezért mindig GMT+0 kerül a bootboot.timezone-ba.
 - Csak a SHA-XOR-CBC titkosítást ismeri, nincs AES
