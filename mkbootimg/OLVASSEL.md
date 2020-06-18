BOOTBOOT Bootolható Lemezkép Készítő
====================================

Általános leírásért lásd a [BOOTBOOT Protokoll](https://gitlab.com/bztsrc/bootboot)t.

Ez egy minden az egyben, többplatformos, függőség nélküli lemezkép kreáló (na jó, zlib kell neki, de az statikusan bele van
forgatva). Egy lemezkonfigurációt kell megadni neki JSON-ben, és létrehozza az ESP FAT boot partíciót a szükséges betöltő
fájlokkal, GPT táblával, PMBR-el, stb. Továbbá képes létrehozni az induló memórialemezképet egy könyvtár tartalmából (jelenleg
`cpio` és `tar` támogatott, de a kód úgy lett megírva, hogy könnyű legyen bővíteni).

A kigenerált képet leellenőriztem fdisk-el, valamint a gdisk verify funkciójával. A FAT partíció tesztelve lett fsck.vfat-al
és UEFI förmverrel, továbbá Raspberry Pi-n.

Működési módok
--------------

Ha az első paraméter `check` (ellenőrzés), akkor a második egy kernel fájlnév. A parancs ellenőrizni fogja a futtathatót,
hogy megfelel-e a BOOTBOOT-nak, részletesen kijelzi a hibákat, és ha átment az ellenőrzésen, megadja, milyen BOOTBOOT
Protokoll szintű betöltő kell a betöltéséhez.

Egyébként az első paraméter a konfigurációs fájl. Ha a második paraméter `initrd.rom`, akkor BIOS Option ROM-ot generál
a megadott initrd könyvtár tartalmából. Ha `bootpart.bin`, akkor a boot partíció képét menti le (és csakis a partíció képét).
Minden más fájlnévre egy teljes lemezképet hoz létre GPT-vel.

Konfiguráció
------------

A JSON egyszerű és rugalmas, többféle variációt is elfogad. A legfelső szinten lehet megadni a lemezre vonatkozó paramétereket.

### Legfelső szint

| Mező       | Típus    | Leírás                                                                              |
|------------|----------|-------------------------------------------------------------------------------------|
| diskguid   | GUID     | opcionális, a lemez GUID-ja. Ha nincs megadva, vagy csupa nulla, akkor generálódik  |
| disksize   | szám     | opcionális, a lemezkép mérete Megabájtban. Ha nincs megadva, kiszámolja             |
| align      | szám     | opcionális, partíció igazítás Kilobájtban. Nullával szektorméretre igazít           |
| iso9660    | logikai  | opcionális, generáljon-e ISO9660 Boot Katalógust a lemezképbe. Alapból ne, false    |
| config     | fájlnév  | a BOOTBOOT konfigurációs fájl. Ebből olvassa ki a kernel fájlnevét                  |
| initrd     | struktúra| az induló lemezkép definícíója, lásd alább                                          |
| partitions | tömb     | a partícíók definíciói, lásd alább                                                  |

Példa:
```
{
    "diskguid": "00000000-0000-0000-0000-000000000000",
    "disksize": 128,
    "align": 1024,
    "iso9660": true,
    "config": "boot/sys/config",
    "initrd": { "type": "tar", "gzip": true, "directory": "boot" },
    "partitions": [
        { "type": "boot", "size": 16 },
        { "type": "Microsoft basic data", "size": 32, "name": "MyOS usr", "file": "usrpart.bin" },
        { "type": "00000000-0000-0000-0000-000000000000", "size": 32, "name": "MyOS var", "file": "varpart.bin" }
    ]
}
```

### Initrd

| Mező       | Típus    | Leírás                                                                              |
|------------|----------|-------------------------------------------------------------------------------------|
| gzip       | logikai  | opcionális, tömörítse-e az induló memórialemezképet, alapértelmezetten igen, true   |
| type       | sztring  | az induló memórialemezkép formátuma. Érvénytelen esetén listázza a lehetőségeket    |
| file       | fájlnév  | a használandó lemezkép fájlneve                                                     |
| directory  | mappa    | mappa elérési útja, a tarmalmából fogja generálni az induló memórialemezképet       |
| file       | tömb     | többarchitektúrás lemezképekhez                                                     |
| directory  | tömb     | többarchitektúrás lemezképekhez                                                     |

A `file` és a `directory` kölcsönösen kizárja egymást. Mindkettő lehet sztring (ha csak egy architektúrához generálunk),
vagy tömb (egy elem minden architektúrához). Jelenleg kettő támogatott, azaz minden tömb maximum két elemű lehet.
Hogy melyik architektúrát jelenti, azt az dönti el, hogy a mabbában vagy lemezképben milyen architektúrájú kernel található.

Példák:
```
    "initrd": { "type": "tar", "gzip": false, "file": "initrd.bin" },
    "initrd": { "type": "tar", "gzip": 1, "directory": "boot" },
    "initrd": { "type": "tar", "gzip": 0, "file": [ "initrd-x86.bin", "initrd-arm.bin" ] },
    "initrd": { "type": "cpio", "gzip": true, "directory": [ "boot/arm", "boot/x86" ] },
```

### Partíciók

Kicsit szokatlan, a legelső elem különbözik a többitől. Az a boot partíciót definiálja, ezért eltérő típusokat
használ, és a `file` valamint a `name` nem használható, mivel az a partíció dinamikusan generált, fix "EFI
System Partition" névvel.

| Mező       | Típus    | Leírás                                                                              |
|------------|----------|-------------------------------------------------------------------------------------|
| size       | szám     | opcionális, a partíció mérete Megabájtban. Ha nincs megadva, kiszámolja             |
| file       | fájlnév  | opcionális, a használandó partíciókép elérési útja                                  |
| type       | sztring  | a partíció formátuma. Érvénytelen esetén listázza a lehetőségeket                   |
| name       | sztring  | UTF-8 partíciónév, korlátozva a 32 és 65535 közötti UNICODE kódpontokra (BMP)       |

Az első elem esetén a `type` lehetséges értékei: `boot` (vagy explicit `fat16` és `fat32`). A parancs igyekszik kényelmesen
használni ezeket, ha lehet FAT16-ot választva, helytakarékosság miatt. A boot partíció minimális mérete 16 Megabájt. Bár mind
a lemezkép készítő, mind a BOOTBOOT betöltő képes lenne kezelni kissebb méretet, néhány UEFI förmver helytelenül FAT12-nek
hiszi, ha túl kevés kluszter van a fájlrendszeren. Ha a partíció mérete meghaladja a 256 Megabájtot, akkor automatikusan
FAT32-t választ. Ha nem használsz `iso9660`-t, akkor kissebb méretű is lehet, de legalább 33 Megabájt (ez a FAT32 minimális
mérete). Ugyanakkor `iso9660` használata esetén garantálni kell, hogy minden kluszter 2048 bájtos címen kezdődjön, amit
4 szektor per kluszterrel a legegyszerűbb elérni. Itt is ugyanaz a probléma merül fel, mind a lemezkép készítő, mind a
BOOTBOOT betöltők képesek lennének kevessebb kluszterrel is használni a FAT32-t, de néhány UEFI förmver nem, és hibásan
FAT16-nak látná. Hogy ezt elkerüljük a minimális kluszterszámmal, az ISO9960 és FAT32 együttes használata esetén a
partíció minimális mérete 256 Megabájt.

A többi (a másodiktól kezdve) bejegyzés esetén a `type` vagy egy GUID, vagy egy az előre definiált aliaszok közül. Érvénytelen
sztring esetén a parancs listázza az összes lehetséges értéket.

Példa:
```
mkbootimg: partition #2 doesn't have a valid type. Accepted values:
  "5A2F534F-0000-5346-2F2F-000000000000" / "FS/Z"
  "6A898CC3-1DD2-11B2-9999-080020736631" / "ZFS"
  "EBD0A0A2-B9E5-4433-8787-68B6B72699C7" / "ntfs"
  "0FC63DAF-8483-4772-8E8E-3D69D8477DE4" / "ext4"
  "516E7CB6-6ECF-11D6-8F8F-00022D09712B" / "ufs"
  "C91818F9-8025-47AF-8989-F030D7000C2C" / "p9"
  "D3BFE2DE-3DAF-11DF-BABA-E3A556D89593" / "Intel Fast Flash"
  "21686148-6449-6E6F-7474-656564454649" / "BIOS boot"
     ...
  "77719A0C-A4A0-11E3-A4A4-000C29745A24" / "VMware Virsto"
  "9198EFFC-31C0-11DB-8F8F-000C2911D1B8" / "VMware Reserved"
  "824CC7A0-36A8-11E3-8989-952519AD3F61" / "OpenBSD data"
  "CEF5A9AD-73BC-4601-8989-CDEEEEE321A1" / "QNX6 file system"
  "C91818F9-8025-47AF-8989-F030D7000C2C" / "Plan 9 partition"
  "5B193300-FC78-40CD-8080-E86C45580B47" / "HiFive Unleashed FSBL"
  "2E54B353-1271-4842-8080-E436D6AF6985" / "HiFive Unleashed BBL"
  ...or any non-zero GUID in the form "%08X-%04X-%04X-%04X-%12X"
```

Ha a `file` meg van adva, akkor a partíció fel lesz tölve a fájl tartalmával. Ha a `size` méret nincs megadva, vagy
kissebb, mint a fájl mérete, akkor a fájl mérete lesz a partíció mérete. Ha mindkettő meg van adva, és a `size` nagyobb,
akkor a kölönbséget nullákkal tölti fel. A partíció mérete mindig `align` Kilobájt többszöröse lesz. 1024 megadásával
a partíciók 1 Megabájtos címekre lesznek igazítva. Az első bejegyzés esetén csak a `size` használható, a `file` nem.

Végezetül a `name` egy sima UTF-8 sztring, a partíció neve.

Újabb fájlrendszerek hozzáadása
-------------------------------

Ezeket az fs registry listázza, az `fs.h` fájlban. Szabadon hozzáadhatsz új típusokat. Azoknál a fájlrendszereknél,
amiket indító memórialemezképhez is szeretnél használni, implementálni kell három funkciót.

```
void cpio_open();
void cpio_add(struct stat *st, char *name, unsigned char *content, int size);
void cpio_close();
```

Az első akkor hívódik, amikor egy új fájlrendszert kell létrehozni. Ahogy a megadott mappát rekurzívan bejárja, minden
almappa és fájl esetén meghívódik az "add". Ez hozzá kell adja a fájlt vagy mappát a fájlrendszer képéhez. Végezetül
amikor a bejárásnak vége, a close hívódik meg, hogy lezárja és véglegesítse a lemezképet.

Ezek a funkciók elérnek két globális változót, az `fs_base`-t és `fs_len`-t, amik a lemezkép memóriabeli bufferét jelölik.

Ezen függvények hiányában, érvényes GPT típussal rendelkezve a fájlrendszer csak a partíciók `type` mezőjében használható.

A beépített binárisok naprakészen tartása
-----------------------------------------

Hogy ne legyen függősége, a lemezkép készítő minden szükséges binárist tartalmaz. Ha ezek frissülnek, akkor le kell törölni
a data.c fájlt, amit a `make` parancs újragenerál. Ha hiányol fájlokat, akkor a `aarch64-rpi` mappában kell kiadni a `make getfw`
parancsot, ami letölti a legfrissebb Raspberry Pi förmver fájlokat. Utánna már menni fog a `make` ebben a könyvtárban.

