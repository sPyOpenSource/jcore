/*
 * x86_64-efi/bootboot.c
 *
 * Copyright (C) 2017 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Booting code for EFI
 *
 */

// DEBUG defined in efidebug.h

#define BBDEBUG 1
//#define GOP_DEBUG BBDEBUG

#if BBDEBUG
#define DBG(fmt, ...) do{Print(fmt,__VA_ARGS__); }while(0);
#else
#define DBG(fmt, ...)
#endif

// get UEFI functions and environment
#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include <efigpt.h>
// get BOOTBOOT specific stuff
#include "../bootboot.h"
#include "tinf.h"
// comment out this include if you don't want FS/Z support
#include "../../osZ/etc/include/fsZ.h"
// get filesystem drivers for initrd
#include "fs.h"

/*** ELF64 defines and structs ***/
#define ELFMAG      "\177ELF"
#define SELFMAG     4
#define EI_CLASS    4       /* File class byte index */
#define ELFCLASS64  2       /* 64-bit objects */
#define EI_DATA     5       /* Data encoding byte index */
#define ELFDATA2LSB 1       /* 2's complement, little endian */
#define PT_LOAD     1       /* Loadable program segment */
#define EM_X86_64   62      /* AMD x86-64 architecture */

typedef struct
{
  unsigned char e_ident[16];/* Magic number and other info */
  UINT16    e_type;         /* Object file type */
  UINT16    e_machine;      /* Architecture */
  UINT32    e_version;      /* Object file version */
  UINT64    e_entry;        /* Entry point virtual address */
  UINT64    e_phoff;        /* Program header table file offset */
  UINT64    e_shoff;        /* Section header table file offset */
  UINT32    e_flags;        /* Processor-specific flags */
  UINT16    e_ehsize;       /* ELF header size in bytes */
  UINT16    e_phentsize;    /* Program header table entry size */
  UINT16    e_phnum;        /* Program header table entry count */
  UINT16    e_shentsize;    /* Section header table entry size */
  UINT16    e_shnum;        /* Section header table entry count */
  UINT16    e_shstrndx;     /* Section header string table index */
} Elf64_Ehdr;

typedef struct
{
  UINT32    p_type;         /* Segment type */
  UINT32    p_flags;        /* Segment flags */
  UINT64    p_offset;       /* Segment file offset */
  UINT64    p_vaddr;        /* Segment virtual address */
  UINT64    p_paddr;        /* Segment physical address */
  UINT64    p_filesz;       /* Segment size in file */
  UINT64    p_memsz;        /* Segment size in memory */
  UINT64    p_align;        /* Segment alignment */
} Elf64_Phdr;

/*** PE32+ defines and structs ***/
#define MZ_MAGIC                    0x5a4d      /* "MZ" */
#define PE_MAGIC                    0x00004550  /* "PE\0\0" */
#define IMAGE_FILE_MACHINE_AMD64    0x8664      /* AMD x86_64 architecture */
#define PE_OPT_MAGIC_PE32PLUS       0x020b      /* PE32+ format */
typedef struct
{
  UINT16 magic;         /* MZ magic */
  UINT16 reserved[29];  /* reserved */
  UINT32 peaddr;        /* address of pe header */
} mz_hdr;

typedef struct {
  UINT32 magic;         /* PE magic */
  UINT16 machine;       /* machine type */
  UINT16 sections;      /* number of sections */
  UINT32 timestamp;     /* time_t */
  UINT32 sym_table;     /* symbol table offset */
  UINT32 symbols;       /* number of symbols */
  UINT16 opt_hdr_size;  /* size of optional header */
  UINT16 flags;         /* flags */
  UINT16 file_type;     /* file type, PE32PLUS magic */
  UINT8  ld_major;      /* linker major version */
  UINT8  ld_minor;      /* linker minor version */
  UINT32 text_size;     /* size of text section(s) */
  UINT32 data_size;     /* size of data section(s) */
  UINT32 bss_size;      /* size of bss section(s) */
  INT32 entry_point;    /* file offset of entry point */
  INT32 code_base;      /* relative code addr in ram */
} pe_hdr;

/*** EFI defines and structs ***/
extern EFI_GUID GraphicsOutputProtocol;
extern EFI_GUID LoadedImageProtocol;
struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
struct EFI_FILE_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
  IN struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *This,
  OUT struct EFI_FILE_PROTOCOL                 **Root
  );

/* Intel EFI headers has simple file protocol, but not GNU EFI */
#ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
  UINT64                                      Revision;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct _EFI_FILE_PROTOCOL {
  UINT64                Revision;
  EFI_FILE_OPEN         Open;
  EFI_FILE_CLOSE        Close;
  EFI_FILE_DELETE       Delete;
  EFI_FILE_READ         Read;
  EFI_FILE_WRITE        Write;
  EFI_FILE_GET_POSITION GetPosition;
  EFI_FILE_SET_POSITION SetPosition;
  EFI_FILE_GET_INFO     GetInfo;
  EFI_FILE_SET_INFO     SetInfo;
  EFI_FILE_FLUSH        Flush;
} EFI_FILE_PROTOCOL;
#endif

#ifndef EFI_PCI_OPTION_ROM_TABLE_GUID
#define EFI_PCI_OPTION_ROM_TABLE_GUID \
  { 0x7462660f, 0x1cbd, 0x48da, {0xad, 0x11, 0x91, 0x71, 0x79, 0x13, 0x83, 0x1c} }
typedef struct {
  EFI_PHYSICAL_ADDRESS   RomAddress; 
  EFI_MEMORY_TYPE        MemoryType;
  UINT32                 RomLength; 
  UINT32                 Seg; 
  UINT8                  Bus; 
  UINT8                  Dev; 
  UINT8                  Func; 
  BOOLEAN                ExecutedLegacyBiosImage; 
  BOOLEAN                DontLoadEfiRom;
} EFI_PCI_OPTION_ROM_DESCRIPTOR;

typedef struct {
  UINT64                         PciOptionRomCount;
  EFI_PCI_OPTION_ROM_DESCRIPTOR   *PciOptionRomDescriptors;
} EFI_PCI_OPTION_ROM_TABLE;
#endif

/*** other defines and structs ***/
typedef struct {
    UINT8 magic[8];
    UINT8 chksum;
    CHAR8 oemid[6];
    UINT8 revision;
    UINT32 rsdt;
    UINT32 length;
    UINT64 xsdt;
    UINT32 echksum;
} __attribute__((packed)) ACPI_RSDPTR;

#define PAGESIZE 4096

/*** common variables ***/
file_t env;         // environment file descriptor
file_t initrd;      // initrd file descriptor
file_t core;        // kernel file descriptor
BOOTBOOT *bootboot; // the BOOTBOOT structure
UINT64 *paging;     // paging table for MMU
UINT64 entrypoint;  // kernel entry point
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
EFI_FILE_HANDLE                 RootDir;
EFI_FILE_PROTOCOL               *Root;
unsigned char *kne;

// default environment variables. M$ states that 1024x768 must be supported
int reqwidth = 1024, reqheight = 768;
char *kernelname="sys/core";

// alternative environment name
char *cfgname="sys/config";

/**
 * function to convert ascii to number
 */
int atoi(unsigned char*c)
{
    int r=0;
    while(*c>='0'&&*c<='9') {
        r*=10; r+=*c-'0';
        c++;
    }
    return r;
}

/**
 * convert ascii to unicode characters
 */
CHAR16 *
a2u (char *str)
{
    static CHAR16 mem[PAGESIZE];
    int i;

    for (i = 0; str[i]; ++i)
        mem[i] = (CHAR16) str[i];
    mem[i] = 0;
    return mem;
}

/**
 * report status with message to standard output
 */
EFI_STATUS
report(EFI_STATUS Status,CHAR16 *Msg)
{
    Print(L"BOOTBOOT-PANIC: %s (EFI %r)\n",Msg,Status);
    return Status;
}

/**
 * convert ascii octal number to binary number
 */
int oct2bin(unsigned char *str,int size)
{
    int s=0;
    unsigned char *c=str;
    while(size-->0){
        s*=8;
        s+=*c-'0';
        c++;
    }
    return s;
}

/**
 * convert ascii hex number to binary number
 */
int hex2bin(unsigned char *str, int size)
{
    int v=0;
    while(size-->0){
        v <<= 4;
        if(*str>='0' && *str<='9')
            v += (int)((unsigned char)(*str)-'0');
        else if(*str >= 'A' && *str <= 'F')
            v += (int)((unsigned char)(*str)-'A'+10);
        str++;
    }
    return v;
}

/**
 * Parse FS0:\BOOTBOOT\CONFIG or /sys/config
 */
EFI_STATUS
ParseEnvironment(unsigned char *cfg, int len, INTN argc, CHAR16 **argv)
{
    unsigned char *ptr=cfg-1;
    int i;
    // failsafe
    if(len>PAGESIZE-1) {
        len=PAGESIZE-1;
    }
    // append temporary variables provided on EFI command line
    // if a key exists multiple times, the last is used
    cfg[len]=0;
    if(argc>2){
        ptr=cfg+len;
        for(i=3;i<argc && ptr+StrLen(argv[i])<cfg+PAGESIZE;i++) {
            CopyMem(ptr,argv[i],StrLen(argv[i]));
            ptr += StrLen(argv[i]);
            *ptr = '\n';
            ptr++;
        }
        *ptr = 0;
        ptr=cfg-1;
    }
    DBG(L" * Environment @%lx %d bytes\n",cfg,len);
    while(ptr<cfg+len) {
        ptr++;
        // failsafe
        if(ptr[0]==0)
            break;
        // skip white spaces
        if(ptr[0]==' '||ptr[0]=='\t'||ptr[0]=='\r'||ptr[0]=='\n')
            continue;
        // skip comments
        if((ptr[0]=='/'&&ptr[1]=='/')||ptr[0]=='#') {
            while(ptr<cfg+len && ptr[0]!='\r' && ptr[0]!='\n' && ptr[0]!=0){
                ptr++;
            }
            ptr--;
            continue;
        }
        if(ptr[0]=='/'&&ptr[1]=='*') {
            ptr+=2;
            while(ptr[0]!=0 && ptr[-1]!='*' && ptr[0]!='/')
                ptr++;
        }
        // parse screen dimensions
        if(!CompareMem(ptr,(const CHAR8 *)"screen=",7)){
            ptr+=7;
            reqwidth=atoi(ptr);
            while(ptr<cfg+len && *ptr!=0 && *(ptr-1)!='x') ptr++;
            reqheight=atoi(ptr);
        }
        // get kernel's filename
        if(!CompareMem(ptr,(const CHAR8 *)"kernel=",7)){
            ptr+=7;
            kernelname=(char*)ptr;
            while(ptr<cfg+len && ptr[0]!='\r' && ptr[0]!='\n' &&
                ptr[0]!=' ' && ptr[0]!='\t' && ptr[0]!=0)
                    ptr++;
            kne=ptr;
            *ptr=0;
            ptr++;
        }
    }
    return EFI_SUCCESS;
}

/**
 * Get a linear frame buffer
 */
EFI_STATUS
GetLFB()
{
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN i, imax, SizeOfInfo, nativeMode, selectedMode=9999, sw=0, sh=0, valid;

    //GOP
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if(EFI_ERROR(status))
        return status;

    // minimum resolution
    if(reqwidth < 640)  reqwidth = 640;
    if(reqheight < 480) reqheight = 480;

    // get current video mode
    status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
    if (status == EFI_NOT_STARTED)
        status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
    if(EFI_ERROR(status))
        return status;
    nativeMode = gop->Mode->Mode;
    imax=gop->Mode->MaxMode;
    for (i = 0; i < imax; i++) {
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
        // failsafe
        if (EFI_ERROR(status))
            continue;
        valid=0;
        // get the mode for the closest resolution
        if((info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor ||
            info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor
// there's a bug in TianoCore, it reports bad masks in PixelInformation, so we don't use PixelBitMask
//          || (info->PixelFormat == PixelBitMask)
           )){
            if(info->HorizontalResolution >= (unsigned int)reqwidth && 
               info->VerticalResolution >= (unsigned int)reqheight &&
               (selectedMode==9999||(info->HorizontalResolution<sw && info->VerticalResolution < sh))) {
                    selectedMode = i;
                    sw = info->HorizontalResolution;
                    sh = info->VerticalResolution;
            }
            valid=1;
        }
        // make gcc happy
        if(valid){}
#if GOP_DEBUG
        DBG(L"    %c%2d %4d x %4d, %d%c ", i==selectedMode?'+':(i==nativeMode?'-':' '),
            i, info->HorizontalResolution, info->VerticalResolution, info->PixelFormat,valid?' ':'?');
        DBG(L"r:%x g:%x b:%x\n",
                info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff:(
                info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff0000:(
                info->PixelFormat==PixelBitMask?info->PixelInformation.RedMask:0)),
                info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor ||
                info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff00:(
                info->PixelFormat==PixelBitMask?info->PixelInformation.GreenMask:0),
                info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff0000:(
                info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff:(
                info->PixelFormat==PixelBitMask?info->PixelInformation.BlueMask:0)));
#endif
    }
    // if we have found a new, better mode
    if(selectedMode != 9999 && selectedMode != nativeMode) {
        status = uefi_call_wrapper(gop->SetMode, 2, gop, selectedMode);
        if(!EFI_ERROR(status))
            nativeMode = selectedMode;
    }
    // get framebuffer properties
    bootboot->fb_ptr=(void*)gop->Mode->FrameBufferBase;
    bootboot->fb_size=gop->Mode->FrameBufferSize;
    bootboot->fb_scanline=4*gop->Mode->Info->PixelsPerScanLine;
    bootboot->fb_width=gop->Mode->Info->HorizontalResolution;
    bootboot->fb_height=gop->Mode->Info->VerticalResolution;
    bootboot->fb_type=
        gop->Mode->Info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor ||
        (gop->Mode->Info->PixelFormat==PixelBitMask && gop->Mode->Info->PixelInformation.BlueMask==0)? FB_ARGB : (
            gop->Mode->Info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor ||
            (gop->Mode->Info->PixelFormat==PixelBitMask && gop->Mode->Info->PixelInformation.RedMask==0)? FB_ABGR : (
                gop->Mode->Info->PixelInformation.BlueMask==0xFF00? FB_RGBA : FB_BGRA
        ));
    DBG(L" * Screen %d x %d, scanline %d, fb @%lx %d bytes, type %d %s\n",
        bootboot->fb_width, bootboot->fb_height, bootboot->fb_scanline,
        bootboot->fb_ptr, bootboot->fb_size, gop->Mode->Info->PixelFormat, 
            bootboot->fb_type==FB_ARGB?L"ARGB":(bootboot->fb_type==FB_ABGR?L"ABGR":(
            bootboot->fb_type==FB_RGBA?L"RGBA":L"BGRA")));
    return EFI_SUCCESS;
}

/**
 * Load a file from FS0 into memory
 */
EFI_STATUS
LoadFile(IN CHAR16 *FileName, OUT UINT8 **FileData, OUT UINTN *FileDataLength)
{
    EFI_STATUS          status;
    EFI_FILE_HANDLE     FileHandle;
    EFI_FILE_INFO       *FileInfo;
    UINT64              ReadSize;
    UINTN               BufferSize;
    UINT8               *Buffer;

    if ((RootDir == NULL) || (FileName == NULL)) {
        return report(EFI_NOT_FOUND,L"Empty Root or FileName\n");
    }

    status = uefi_call_wrapper(RootDir->Open, 5, RootDir, &FileHandle, FileName, 
        EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (EFI_ERROR(status)) {
        return status;
//        Print(L"%s not found\n",FileName);
//        return report(status,L"Open error");
    }
    FileInfo = LibFileInfo(FileHandle);
    if (FileInfo == NULL) {
        uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
        Print(L"%s not found\n",FileName);
        return report(EFI_NOT_FOUND,L"FileInfo error");
    }
    ReadSize = FileInfo->FileSize;
    if (ReadSize > 16*1024*1024)
        ReadSize = 16*1024*1024;
    FreePool(FileInfo);

    BufferSize = (UINTN)((ReadSize+PAGESIZE-1)/PAGESIZE);
    status = uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, BufferSize, (EFI_PHYSICAL_ADDRESS*)&Buffer);
    if (Buffer == NULL) {
        uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
        return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
    }
    status = uefi_call_wrapper(FileHandle->Read, 3, FileHandle, &ReadSize, Buffer);
    uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
    if (EFI_ERROR(status)) {
        uefi_call_wrapper(BS->FreePages, 2, (EFI_PHYSICAL_ADDRESS)(Buffer), BufferSize);
        Print(L"%s not found\n",FileName);
        return report(status,L"Read error");
    }

    *FileData = Buffer;
    *FileDataLength = ReadSize;
    return EFI_SUCCESS;
}

/**
 * Locate and load the kernel in initrd
 */
EFI_STATUS
LoadCore()
{
    int i=0,bss=0;
    UINT8 *ptr;
    core.ptr=ptr=NULL;
    while(core.ptr==NULL && fsdrivers[i]!=NULL) {
        core=(*fsdrivers[i++])((unsigned char*)initrd.ptr,kernelname);
    }
    // if every driver failed, try brute force, scan for the first elf or pe executable
    if(core.ptr==NULL) {
        DBG(L" * Autodetecting kernel%s\n","");
        i=initrd.size;
        core.ptr=initrd.ptr;
        while(i-->0) {
            Elf64_Ehdr *ehdr=(Elf64_Ehdr *)(core.ptr);
            pe_hdr *pehdr=(pe_hdr*)(core.ptr + ((mz_hdr*)(core.ptr))->peaddr);
            if((!CompareMem(ehdr->e_ident,ELFMAG,SELFMAG)||!CompareMem(ehdr->e_ident,"OS/Z",4))&&
                ehdr->e_ident[EI_CLASS]==ELFCLASS64&&
                ehdr->e_ident[EI_DATA]==ELFDATA2LSB&&
                ehdr->e_machine==EM_X86_64&&
                ehdr->e_phnum>0){
                    break;
                }
            if(((mz_hdr*)(core.ptr))->magic==MZ_MAGIC && pehdr->magic == PE_MAGIC && 
                pehdr->machine == IMAGE_FILE_MACHINE_AMD64 && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS) {
                    break;
                }
            core.ptr++;
        }
        core.ptr=NULL;
    }

    if(core.ptr!=NULL) {
        Elf64_Ehdr *ehdr=(Elf64_Ehdr *)(core.ptr);
        pe_hdr *pehdr=(pe_hdr*)(core.ptr + ((mz_hdr*)(core.ptr))->peaddr);
        if((!CompareMem(ehdr->e_ident,ELFMAG,SELFMAG)||!CompareMem(ehdr->e_ident,"OS/Z",4))&&
            ehdr->e_ident[EI_CLASS]==ELFCLASS64&&ehdr->e_ident[EI_DATA]==ELFDATA2LSB&&
            ehdr->e_machine==EM_X86_64&&ehdr->e_phnum>0){
            // Parse ELF64
            DBG(L" * Parsing ELF64 @%lx\n",core.ptr);
            Elf64_Phdr *phdr=(Elf64_Phdr *)((UINT8 *)ehdr+ehdr->e_phoff);
            for(i=0;i<ehdr->e_phnum;i++){
                if(phdr->p_type==PT_LOAD && phdr->p_vaddr>>48==0xffff) {
                    // hack to keep symtab and strtab for shared libraries
                    core.size = phdr->p_filesz + (ehdr->e_type==3?0x4000:0);
                    ptr = (UINT8*)ehdr + phdr->p_offset;
                    bss = phdr->p_memsz - core.size;
                    entrypoint = ehdr->e_entry;
                    break;
                }
                phdr=(Elf64_Phdr *)((UINT8 *)phdr+ehdr->e_phentsize);
            }
        } else if(((mz_hdr*)(core.ptr))->magic==MZ_MAGIC && pehdr->magic == PE_MAGIC && 
            pehdr->machine == IMAGE_FILE_MACHINE_AMD64 && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS &&
            (INT64)pehdr->code_base>>48==0xffff) {
                //Parse PE32+
                DBG(L" * Parsing PE32+ @%lx\n",core.ptr);
                core.size = (pehdr->entry_point-pehdr->code_base) + pehdr->text_size + pehdr->data_size;
                ptr = core.ptr;
                bss = pehdr->bss_size;
                entrypoint = (INT64)pehdr->entry_point;
        }
        if(ptr==NULL || core.size<2 || entrypoint==0)
            return report(EFI_LOAD_ERROR,L"Kernel is not a valid executable");
        // create core segment
        uefi_call_wrapper(BS->AllocatePages, 4, 0, 2,
            (core.size + bss + PAGESIZE-1)/PAGESIZE, (EFI_PHYSICAL_ADDRESS*)&core.ptr);
        if (core.ptr == NULL)
            return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
        CopyMem((void*)core.ptr,ptr,core.size);
        if(bss>0)
            ZeroMem((void*)core.ptr + core.size, bss); 
        core.size += bss;
        DBG(L" * Entry point @%lx, text @%lx %d bytes\n",entrypoint, core.ptr, core.size);
        core.size = ((core.size+PAGESIZE-1)/PAGESIZE)*PAGESIZE;
        return EFI_SUCCESS;

    }
    return report(EFI_LOAD_ERROR,L"Kernel not found in initrd");
}

/**
 * Main EFI application entry point
 */
EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lipGuid = LOADED_IMAGE_PROTOCOL;
    EFI_GUID RomTableGuid = EFI_PCI_OPTION_ROM_TABLE_GUID;
    EFI_PCI_OPTION_ROM_TABLE *RomTable;
    EFI_GUID bioGuid = BLOCK_IO_PROTOCOL;
    EFI_BLOCK_IO *bio;
    EFI_HANDLE *handles = NULL;
    EFI_STATUS status=EFI_SUCCESS;
    EFI_MEMORY_DESCRIPTOR *memory_map = NULL, *mement;
    EFI_PARTITION_TABLE_HEADER *gptHdr;
    EFI_PARTITION_ENTRY *gptEnt;
    EFI_INPUT_KEY key;
    UINTN i, j=0, handle_size=0,memory_map_size=0, map_key=0, desc_size=0;
    UINT32 desc_version=0;
    UINT64 lba_s=0,lba_e=0;
    MMapEnt *mmapent, *last=NULL;
    file_t ret={NULL,0};
    CHAR16 **argv, *initrdfile, *configfile, *help=
        L"SYNOPSIS\n  BOOTBOOT.EFI [ -h | -? | /h | /? ] [ INITRDFILE [ ENVIRONMENTFILE [...] ] ]\n\nDESCRIPTION\n  Bootstraps an operating system via the BOOTBOOT Protocol.\n  If arguments not given, defaults to\n    FS0:\\BOOTBOOT\\INITRD   as ramdisk image and\n    FS0:\\BOOTBOOT\\CONFIG   for boot environment.\n  Additional \"key=value\" command line arguments will be appended to the\n  environment. If INITRD not found, it will use the first bootable partition\n  in GPT. If CONFIG not found, it will look for /sys/config inside the\n  INITRD (or partition).\n\n  As this is a loader, it is not supposed to return control to the shell.\n\n";
    INTN argc;

    // Initialize UEFI Library
    InitializeLib(image, systab);
    BS = systab->BootServices;

    // Parse command line arguments
    // BOOTBOOT.EFI [-?|-h|/?|/h] [initrd [config [key=value...]]
    argc = GetShellArgcArgv(image, &argv);
    if(argc>1) {
        if((argv[1][0]=='-'||argv[1][0]=='/')&&(argv[1][1]=='?'||argv[1][1]=='h')){
            Print(L"BOOTBOOT LOADER (build %s)\n\n%s",a2u(__DATE__),help);
            return EFI_SUCCESS;
        }
        initrdfile=argv[1];
    } else {
        initrdfile=L"\\BOOTBOOT\\INITRD";
    }
    if(argc>2) {
        configfile=argv[2];
    } else {
        configfile=L"\\BOOTBOOT\\CONFIG";
    }

    Print(L"Booting OS...\n");

    // get memory for bootboot structure
    uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 1, (EFI_PHYSICAL_ADDRESS*)&bootboot);
    if (bootboot == NULL)
        return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
    ZeroMem((void*)bootboot,PAGESIZE);
    CopyMem(bootboot->magic,BOOTBOOT_MAGIC,4);
    // unlike BIOS+MultiBoot bootboot, no need to check if we have
    // PAE + MSR + LME, as we're already in long mode.
    __asm__ __volatile__ (
        "mov $1, %%eax;"
        "cpuid;"
        "shrl $24, %%ebx;"
        "mov %%bx,%0"
        : "=b"(bootboot->bspid) : : );

    // locate InitRD in ROM
    DBG(L" * Locate initrd in Option ROMs%s\n",L"");
    RomTable = NULL; initrd.ptr = NULL; initrd.size = 0;
    status=EFI_LOAD_ERROR;
    // first, try RomTable
    LibGetSystemConfigurationTable(&RomTableGuid,(void *)&(RomTable));
    if(RomTable!=NULL) {
        for (i=0;i<RomTable->PciOptionRomCount;i++) {
            ret.ptr=(UINT8*)RomTable->PciOptionRomDescriptors[i].RomAddress;
            if(ret.ptr[0]==0x55 && ret.ptr[1]==0xAA && !CompareMem(ret.ptr+8,(const CHAR8 *)"INITRD",6)) {
                CopyMem(&initrd.size,ret.ptr+16,4);
                initrd.ptr=ret.ptr+32;
                status=EFI_SUCCESS;
                break;
            }
        }
    }
    //if not found, scan memory
    if(EFI_ERROR(status) || initrd.ptr==NULL){
        status = uefi_call_wrapper(BS->GetMemoryMap, 5,
            &memory_map_size, memory_map, NULL, &desc_size, NULL);
        if (status!=EFI_BUFFER_TOO_SMALL || memory_map_size==0) {
            return report(EFI_OUT_OF_RESOURCES,L"GetMemoryMap getSize");
        }
        memory_map_size+=2*desc_size;
        uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 
            (memory_map_size+PAGESIZE-1)/PAGESIZE, 
            (EFI_PHYSICAL_ADDRESS*)&memory_map);
        if (memory_map == NULL) {
            return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
        }
        status = uefi_call_wrapper(BS->GetMemoryMap, 5,
            &memory_map_size, memory_map, &map_key, &desc_size, &desc_version);
        status=EFI_LOAD_ERROR;
        for(mement=memory_map;
            mement<memory_map+memory_map_size;
            mement=NextMemoryDescriptor(mement,desc_size)) {
                if(mement==NULL || (mement->PhysicalStart==0 && mement->NumberOfPages==0))
                    break;
                // skip free and ACPI memory
                if(mement->Type==7||mement->Type==9||mement->Type==10)
                    continue;
                // according to spec, EFI Option ROMs must start on 512 bytes boundary, not 2048
                for(ret.ptr=(UINT8*)mement->PhysicalStart;
                    ret.ptr<(UINT8*)mement->PhysicalStart+mement->NumberOfPages*PAGESIZE;
                    ret.ptr+=512) {
                    if(ret.ptr[0]==0x55 && ret.ptr[1]==0xAA && !CompareMem(ret.ptr+8,(const CHAR8 *)"INITRD",6)) {
                        CopyMem(&initrd.size,ret.ptr+16,4);
                        initrd.ptr=ret.ptr+32;
                        status=EFI_SUCCESS;
                        goto foundinrom;
                    }
                }
        }
foundinrom:
        uefi_call_wrapper(BS->FreePages, 2, (EFI_PHYSICAL_ADDRESS)memory_map, (memory_map_size+PAGESIZE-1)/PAGESIZE);
    }
    // fall back to INITRD on filesystem
    if(EFI_ERROR(status) || initrd.ptr==NULL){
        // if the user presses any key now, we fallback to backup initrd
        for(i=0;i<50;i++) {
            // delay 5ms
            uefi_call_wrapper(BS->Stall, 1, 5000);
            status=uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key);
            if(status!=EFI_NOT_READY) {
                Print(L" * Backup initrd\n");
                initrdfile=L"\\BOOTBOOT\\INITRD.BAK";
                break;
            }
        }
        DBG(L" * Locate initrd in %s\n",initrdfile);
        // Initialize FS with the DeviceHandler from loaded image protocol
        status = uefi_call_wrapper(BS->HandleProtocol,
                    3,
                    image,
                    &lipGuid,
                    (void **) &loaded_image);
        if (!EFI_ERROR(status) && loaded_image!=NULL) {
            status=EFI_LOAD_ERROR;
            RootDir = LibOpenRoot(loaded_image->DeviceHandle);
            // load ramdisk
            status=LoadFile(initrdfile,&initrd.ptr, &initrd.size);
        }
    }
    // if not found, try architecture specific initrd file
    if(EFI_ERROR(status) || initrd.ptr==NULL){
        initrdfile=L"\\BOOTBOOT\\X86_64";
        DBG(L" * Locate initrd in %s\n",initrdfile);
        status=LoadFile(initrdfile,&initrd.ptr, &initrd.size);
    }
    // if even that failed, look for a partition
    if(status!=EFI_SUCCESS || initrd.size==0){
        DBG(L" * Locate initrd in GPT%s\n",L"");
        status = uefi_call_wrapper(BS->LocateHandle, 5, ByProtocol, &bioGuid, NULL, &handle_size, handles);
        if (status!=EFI_BUFFER_TOO_SMALL || handle_size==0) {
            return report(EFI_OUT_OF_RESOURCES,L"LocateHandle getSize");
        }
        uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, (handle_size+PAGESIZE-1)/PAGESIZE, (EFI_PHYSICAL_ADDRESS*)&handles);
        if(handles==NULL)
            return report(EFI_OUT_OF_RESOURCES,L"AllocatePages\n");
        uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 1, (EFI_PHYSICAL_ADDRESS*)&initrd.ptr);
        if (initrd.ptr == NULL)
            return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
        lba_s=lba_e=0;
        status = uefi_call_wrapper(BS->LocateHandle, 5, ByProtocol, &bioGuid, NULL, &handle_size, handles);
        for(i=0;i<handle_size/sizeof(EFI_HANDLE);i++) {
            // we have to do it the hard way. HARDDRIVE_DEVICE_PATH does not return partition type or attribs...
            status = uefi_call_wrapper(BS->HandleProtocol, 3, handles[i], &bioGuid, (void **) &bio);
            if(status!=EFI_SUCCESS || bio==NULL || bio->Media->BlockSize==0)
                continue;
            status=bio->ReadBlocks(bio, bio->Media->MediaId, 1, PAGESIZE, initrd.ptr);
            if(status!=EFI_SUCCESS || CompareMem(initrd.ptr,EFI_PTAB_HEADER_ID,8))
                continue;
            gptHdr = (EFI_PARTITION_TABLE_HEADER*)initrd.ptr;
            if(gptHdr->NumberOfPartitionEntries>127) gptHdr->NumberOfPartitionEntries=127;
            // first, look for a partition with bootable flag
            ret.ptr= (UINT8*)(initrd.ptr + (gptHdr->PartitionEntryLBA-1) * bio->Media->BlockSize);
            for(j=0;j<gptHdr->NumberOfPartitionEntries;j++) {
                gptEnt=(EFI_PARTITION_ENTRY*)ret.ptr;
                if((ret.ptr[0]==0 && ret.ptr[1]==0 && ret.ptr[2]==0 && ret.ptr[3]==0) || gptEnt->StartingLBA==0)
                    break;
                // use first partition with bootable flag as INITRD
                if((gptEnt->Attributes & EFI_PART_USED_BY_OS)) goto partfound;
                ret.ptr+=gptHdr->SizeOfPartitionEntry;
            }
            // if none, look for specific partition types
            ret.ptr= (UINT8*)(initrd.ptr + (gptHdr->PartitionEntryLBA-1) * bio->Media->BlockSize);
            for(j=0;j<gptHdr->NumberOfPartitionEntries;j++) {
                gptEnt=(EFI_PARTITION_ENTRY*)ret.ptr;
                if((ret.ptr[0]==0 && ret.ptr[1]==0 && ret.ptr[2]==0 && ret.ptr[3]==0) || gptEnt->StartingLBA==0)
                    break;
                    // use the first OS/Z root partition for this architecture
                if(!CompareMem(&gptEnt->PartitionTypeGUID.Data1,"OS/Z",4) &&
                    gptEnt->PartitionTypeGUID.Data2==0x8664 && 
                    !CompareMem(&gptEnt->PartitionTypeGUID.Data4[4],"root",4)) {
partfound:              lba_s=gptEnt->StartingLBA; lba_e=gptEnt->EndingLBA;
                        initrd.size = (((lba_e-lba_s)*bio->Media->BlockSize + PAGESIZE-1)/PAGESIZE)*PAGESIZE;
                        status=EFI_SUCCESS;
                        goto partok;
                }
                ret.ptr+=gptHdr->SizeOfPartitionEntry;
            }
        }
        return report(EFI_LOAD_ERROR,L"No boot partition");
partok:
        uefi_call_wrapper(BS->FreePages, 2, (EFI_PHYSICAL_ADDRESS)initrd.ptr, 1);
        if(initrd.size>0 && bio!=NULL) {
            uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, initrd.size/PAGESIZE, (EFI_PHYSICAL_ADDRESS*)&initrd.ptr);
            if (initrd.ptr == NULL)
                return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
            status=bio->ReadBlocks(bio, bio->Media->MediaId, lba_s, initrd.size, initrd.ptr);            
        } else
            status=EFI_LOAD_ERROR;
    }
    if(status==EFI_SUCCESS && initrd.size>0){
        //check if initrd is gzipped
        if(initrd.ptr[0]==0x1f && initrd.ptr[1]==0x8b){
            unsigned char *addr,f;
            int len=0, r;
            TINF_DATA d;
            DBG(L" * Gzip compressed initrd @%lx %d bytes\n",initrd.ptr,initrd.size);
            // skip gzip header
            addr=initrd.ptr+2;
            if(*addr++!=8) goto gzerr;
            f=*addr++; addr+=6;
            if(f&4) { r=*addr++; r+=(*addr++ << 8); addr+=r; }
            if(f&8) { while(*addr++ != 0); }
            if(f&16) { while(*addr++ != 0); }
            if(f&2) addr+=2;
            d.source = addr;
            // allocate destination buffer
            CopyMem(&len,initrd.ptr+initrd.size-4,4);
            uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, (len+PAGESIZE-1)/PAGESIZE, (EFI_PHYSICAL_ADDRESS*)&addr);
            if(addr==NULL)
                return report(EFI_OUT_OF_RESOURCES,L"AllocatePages\n");
            // decompress
            d.bitcount = 0;
            d.bfinal = 0;
            d.btype = -1;
            d.dict_size = 0;
            d.dict_ring = NULL;
            d.dict_idx = 0;
            d.curlen = 0;
            d.dest = addr;
            d.destSize = len;
            do { r = uzlib_uncompress(&d); } while (!r);
            if (r != TINF_DONE) {
gzerr:          return report(EFI_COMPROMISED_DATA,L"Unable to uncompress");
            }
            // swap initrd.ptr with the uncompressed buffer
            // if it's not page aligned, we came from ROM, no FreePages
            if(((UINT64)initrd.ptr&(PAGESIZE-1))==0)
                uefi_call_wrapper(BS->FreePages, 2, (EFI_PHYSICAL_ADDRESS)initrd.ptr, (initrd.size+PAGESIZE-1)/PAGESIZE);
            initrd.ptr=addr;
            initrd.size=len;
        }
        DBG(L" * Initrd loaded @%lx %d bytes\n",initrd.ptr,initrd.size);
        kne=env.ptr=NULL;
        // if there's an environment file, load it
        if(loaded_image!=NULL && LoadFile(configfile,&env.ptr,&env.size)!=EFI_SUCCESS) {
            env.ptr=NULL;
        }
        if(env.ptr==NULL) {
            // if there were no environment file on boot partition, find it inside the INITRD
            j=0; ret.ptr=NULL; ret.size=0;
            while(ret.ptr==NULL && fsdrivers[j]!=NULL) {
                ret=(*fsdrivers[j++])((unsigned char*)initrd.ptr,cfgname);
            }
            if(ret.ptr!=NULL) {
                if(ret.size>PAGESIZE-1)
                    ret.size=PAGESIZE-1;
                uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 1, (EFI_PHYSICAL_ADDRESS*)&env.ptr);
                if(env.ptr==NULL)
                    return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
                ZeroMem((void*)env.ptr,PAGESIZE);
                CopyMem((void*)env.ptr,ret.ptr,ret.size);
                env.size=ret.size;
            }
        }
        if(env.ptr!=NULL) {
            ParseEnvironment(env.ptr,env.size, argc, argv);
        } else {
            // provide an empty environment for the OS
            env.size=0;
            uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 1, (EFI_PHYSICAL_ADDRESS*)&env.ptr);
            if (env.ptr == NULL) {
                return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
            }
            ZeroMem((void*)env.ptr,PAGESIZE);
            CopyMem((void*)env.ptr,"// N/A",8);
        }

        // get linear frame buffer
        status = GetLFB();
        if (EFI_ERROR(status) || bootboot->fb_width==0 || bootboot->fb_ptr==0)
                return report(status, L"GOP failed, no framebuffer");

        // collect information on system
        bootboot->protocol=PROTOCOL_STATIC;
        bootboot->loader_type=LOADER_UEFI;
        bootboot->size=128;
        bootboot->pagesize=12;
        CopyMem((void *)&(bootboot->initrd_ptr),&initrd.ptr,8);
        bootboot->initrd_size=((initrd.size+PAGESIZE-1)/PAGESIZE)*PAGESIZE;
        CopyMem((void *)&(bootboot->x86_64.efi_ptr),&systab,8);

        // System tables and structures
        DBG(L" * System tables%s\n","");
        LibGetSystemConfigurationTable(&AcpiTableGuid,(void *)&(bootboot->x86_64.acpi_ptr));
        LibGetSystemConfigurationTable(&SMBIOSTableGuid,(void *)&(bootboot->x86_64.smbi_ptr));
        LibGetSystemConfigurationTable(&MpsTableGuid,(void *)&(bootboot->x86_64.mp_ptr));

        // FIX ACPI table pointer on TianoCore...
        ret.ptr = (UINT8*)(bootboot->x86_64.acpi_ptr);
        if(CompareMem(ret.ptr,(const CHAR8 *)"RSDT", 4) && CompareMem(ret.ptr,(const CHAR8 *)"XSDT", 4)) {
            // scan for the real rsd ptr, as AcpiTableGuid returns bad address
            for(i=1;i<256;i++) {
                if(!CompareMem(ret.ptr+i, (const CHAR8 *)"RSD PTR ", 8)){
                    ret.ptr+=i;
                    break;
                }
            }
            // get ACPI system table
            ACPI_RSDPTR *rsd = (ACPI_RSDPTR*)ret.ptr;
            if(rsd->xsdt!=0)
                bootboot->x86_64.acpi_ptr = rsd->xsdt;
            else
                bootboot->x86_64.acpi_ptr = (UINT64)((UINT32)rsd->rsdt);
        }

        // Date and time
        EFI_TIME t;
        uefi_call_wrapper(ST->RuntimeServices->GetTime, 2, &t, NULL);
        bootboot->datetime[0]=DecimaltoBCD(t.Year/100);
        bootboot->datetime[1]=DecimaltoBCD(t.Year%100);
        bootboot->datetime[2]=DecimaltoBCD(t.Month);
        bootboot->datetime[3]=DecimaltoBCD(t.Day);
        bootboot->datetime[4]=DecimaltoBCD(t.Hour);
        bootboot->datetime[5]=DecimaltoBCD(t.Minute);
        bootboot->datetime[6]=DecimaltoBCD(t.Second);
        bootboot->datetime[7]=DecimaltoBCD(t.Daylight);
        CopyMem((void *)&bootboot->timezone, &t.TimeZone, 2);
        if(bootboot->timezone<-1440||bootboot->timezone>1440)   // TZ in mins
            bootboot->timezone=0;
        DBG(L" * System time %d-%02d-%02d %02d:%02d:%02d GMT%s%d:%02d %s\n",
            t.Year,t.Month,t.Day,t.Hour,t.Minute,t.Second,
            bootboot->timezone>=0?L"+":L"",bootboot->timezone/60,bootboot->timezone%60,
            t.Daylight?L"summertime":L"");
        // get sys/core and parse
        status=LoadCore();
        if (EFI_ERROR(status))
            return status;
        if(kne!=NULL)
            *kne='\n';

        // query size of memory map
        status = uefi_call_wrapper(BS->GetMemoryMap, 5,
            &memory_map_size, memory_map, NULL, &desc_size, NULL);
        if (status!=EFI_BUFFER_TOO_SMALL || memory_map_size==0) {
            return report(EFI_OUT_OF_RESOURCES,L"GetMemoryMap getSize");
        }
        // allocate memory for memory descriptors. We assume that one or two new memory
        // descriptor may created by our next allocate calls and we round up to page size
        memory_map_size+=2*desc_size;
        uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 
            (memory_map_size+PAGESIZE-1)/PAGESIZE, 
            (EFI_PHYSICAL_ADDRESS*)&memory_map);
        if (memory_map == NULL) {
            return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
        }

        // create page tables
        uefi_call_wrapper(BS->AllocatePages, 4, 0, 2, 24, (EFI_PHYSICAL_ADDRESS*)&paging);
        if (paging == NULL) {
            return report(EFI_OUT_OF_RESOURCES,L"AllocatePages");
        }
        ZeroMem((void*)paging,23*PAGESIZE);
        DBG(L" * Pagetables PML4 @%lx\n",paging);
        //PML4
        paging[0]=(UINT64)((UINT8 *)paging+4*PAGESIZE)+1;   // pointer to 2M PDPE (16G RAM identity mapped)
        paging[511]=(UINT64)((UINT8 *)paging+PAGESIZE)+1;   // pointer to 4k PDPE (core mapped at -2M)
        //4k PDPE
        paging[512+511]=(UINT64)((UINT8 *)paging+2*PAGESIZE+1);
        //4k PDE
        for(i=0;i<31;i++)
            paging[2*512+480+i]=(UINT64)(((UINT8 *)(bootboot->fb_ptr)+(i<<21))+0x81);   //map framebuffer
        paging[2*512+511]=(UINT64)((UINT8 *)paging+3*PAGESIZE+1);
        //4k PT
        paging[3*512+0]=(UINT64)(bootboot)+1;
        paging[3*512+1]=(UINT64)(env.ptr)+1;
        for(i=0;i<(core.size/PAGESIZE);i++)
            paging[3*512+2+i]=(UINT64)((UINT8 *)core.ptr+i*PAGESIZE+1);
        paging[3*512+511]=(UINT64)((UINT8 *)paging+23*PAGESIZE+1);  // core stack
        //identity mapping
        //2M PDPE
        for(i=0;i<16;i++)
            paging[4*512+i]=(UINT64)((UINT8 *)paging+(7+i)*PAGESIZE+1);
        //first 2M mapped per page
        paging[7*512]=(UINT64)((UINT8 *)paging+5*PAGESIZE+1);
        for(i=0;i<512;i++)
            paging[5*512+i]=(UINT64)(i*PAGESIZE+1);
        //2M PDE
        for(i=1;i<512*16;i++)
            paging[7*512+i]=(UINT64)((i<<21)+0x81);

        // Get memory map
        int cnt=3;
get_memory_map:
        DBG(L" * Memory Map @%lx %d bytes #%d\n",memory_map, memory_map_size, 4-cnt);
        mmapent=(MMapEnt *)&(bootboot->mmap);
        status = uefi_call_wrapper(BS->GetMemoryMap, 5,
            &memory_map_size, memory_map, &map_key, &desc_size, &desc_version);
        if (EFI_ERROR(status)) {
            return report(status,L"GetMemoryMap");
        }
        last=NULL;
        for(mement=memory_map;
            mement<memory_map+memory_map_size;
            mement=NextMemoryDescriptor(mement,desc_size)) {
            // failsafe
            if(mement==NULL || bootboot->size>=PAGESIZE-128 || 
                (mement->PhysicalStart==0 && mement->NumberOfPages==0))
                break;
            // failsafe, don't report our own structures as free
            if( mement->NumberOfPages==0 ||
                ((mement->PhysicalStart <= (UINT64)bootboot &&
                    mement->PhysicalStart+(mement->NumberOfPages*PAGESIZE) > (UINT64)bootboot) ||
                 (mement->PhysicalStart <= (UINT64)env.ptr &&
                    mement->PhysicalStart+(mement->NumberOfPages*PAGESIZE) > (UINT64)env.ptr) ||
                 (mement->PhysicalStart <= (UINT64)initrd.ptr &&
                    mement->PhysicalStart+(mement->NumberOfPages*PAGESIZE) > (UINT64)initrd.ptr) ||
                 (mement->PhysicalStart <= (UINT64)core.ptr &&
                    mement->PhysicalStart+(mement->NumberOfPages*PAGESIZE) > (UINT64)core.ptr) ||
                 (mement->PhysicalStart <= (UINT64)paging &&
                    mement->PhysicalStart+(mement->NumberOfPages*PAGESIZE) > (UINT64)paging)
                )) {
                    continue;
            }
            mmapent->ptr=mement->PhysicalStart;
            mmapent->size=(mement->NumberOfPages*PAGESIZE)+
                ((mement->Type>0&&mement->Type<5)||mement->Type==7?MMAP_FREE:
                (mement->Type==9?MMAP_ACPIFREE:
                (mement->Type==10?MMAP_ACPINVS:
                (mement->Type==11||mement->Type==12?MMAP_MMIO:
                MMAP_USED))));
            // merge continous areas of the same type
            if(last!=NULL && 
                MMapEnt_Type(last) == MMapEnt_Type(mmapent) &&
                MMapEnt_Ptr(last)+MMapEnt_Size(last) == MMapEnt_Ptr(mmapent)) {
                    last->size+=MMapEnt_Size(mmapent);
                    mmapent->ptr=mmapent->size=0;
            } else {
                last=mmapent;
                bootboot->size+=16;
                mmapent++;
            }
        }
        // --- NO PRINT AFTER THIS POINT ---

        //inform firmware that we're about to leave it's realm
        status = uefi_call_wrapper(BS->ExitBootServices, 2, image, map_key);
        if(EFI_ERROR(status)){
            cnt--;
            if(cnt>0) goto get_memory_map;
            return report(status,L"ExitBootServices");
        }

        //set up paging
        __asm__ __volatile__ (
            "mov %0,%%rax;"
            "mov %%rax,%%cr3"
            : : "b"(paging) : "memory" );

        //call _start() in sys/core
        __asm__ __volatile__ (
            "xorq %%rsp, %%rsp;"
            "pushq %0;"
            "retq"
            : : "a"(entrypoint): "memory" );
    }
    return report(status,L"INITRD not found");
}

