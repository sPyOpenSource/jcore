// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://codeberg.org/ThatOSDev/EFI_AARCH64

#include "efi.h"
#include "ErrorCodes.h"
#include "efilibs.h"
#include "all.h"

/*#define EFI_FILE_INFO_ID \
  { \
    0x9576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b } \
  }

UINT64* FileSize(EFI_FILE_PROTOCOL *FileHandle)
{
  UINT64* FileSize = 0;
  FileHandle->SetPosition(FileHandle, 0xFFFFFFFFFFFFFFFFULL);
  FileHandle->GetPosition(FileHandle, FileSize);
  return FileSize;
}*/
void loadIt(DomainDesc * domain, char *libname, char* codefilepos);

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
	InitEFI(image, system);

    EFI_PHYSICAL_ADDRESS ExternalFileBuffer = 0;
    EFI_PHYSICAL_ADDRESS ExternalFileBuffer1 = 0;

    EFI_FILE_PROTOCOL* efimyfile = openFile(u"testfile.bin");
    EFI_FILE_PROTOCOL* zero = openFile(u"zero.jll");
    //EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;
	UINTN FileInfoSize = 0x00001000;
    EFI_FILE_INFO *FileInfo;
    EFI_ALLOCATE_POOL AllocatePool = SystemTable->BootServices->AllocatePool;
    EFI_STATUS Status;// = zero->GetInfo(zero, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    AllocatePool(EfiLoaderCode, FileInfoSize, (void**)&ExternalFileBuffer1);
    //AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    //Status = zero->GetInfo(zero, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);
    /*UINT64* FileSize = 0;
	//efimyfile->SetPosition(efimyfile, 0xFFFFFFFFFFFFFFFFULL);
	efimyfile->SetPosition(efimyfile, 0);
    efimyfile->GetPosition(efimyfile, FileSize);*/

    Status = AllocatePool(EfiLoaderCode, FileInfoSize, (void**)&ExternalFileBuffer);

	SetTextColor(EFI_BROWN);
    wprintf(u"AllocatePool ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));

    efimyfile->SetPosition(efimyfile, 0);
    efimyfile->Read(efimyfile, &FileInfoSize, (void *)ExternalFileBuffer);

zero->SetPosition(zero, 0);
zero->Read(zero, &FileInfoSize, (void *)ExternalFileBuffer1);

    SetTextColor(EFI_GREEN);
    wprintf(u"Read ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));

    SetTextColor(EFI_LIGHTCYAN);
    //wprintf(u"\r\nFirst 5 Bytes\r\n");
    SetTextColor(EFI_LIGHTRED);
    char* codefile = (char*)ExternalFileBuffer1;

    /*for(int m = 0; m < 5; m++)
    {
        int j = *codefile;
        wprintf(u"%x ", j);
        codefile++;
    }*/
	loadIt(NULL, "zero.jll", codefile);
	efimyfile->SetPosition(efimyfile, 0);
    
    SetTextColor(EFI_GREEN);
    SetTextPosition(10, 20);
    wprintf(u"Hit Any Key to execute testfile");

    HitAnyKey();
	
	SetGraphicsColor(BLACK);
	
	CreateFilledBox(150, 150, 800, 500);
	
	SetGraphicsColor(ORANGE);
	
	// Execute File, get return number --> 349587 \ 055593
	int (*KernelBinFile)(void) = (int (*)(void)) ((UINT8 *)ExternalFileBuffer);

    int g = KernelBinFile();

    SetTextColor(EFI_LIGHTMAGENTA);
    wprintf(u"\r\nThe returned number is : %d", g);

    SetTextColor(EFI_GREEN);
    SetTextPosition(10, 13);
    wprintf(u"Hit Any Key to shutdown");

    HitAnyKey();
	
	SHUTDOWN();
	
    SetTextColor(EFI_GREEN);
    SetTextPosition(0, 22);
    wprintf(u"If you see this, something went wrong. Manually turn off the computer.");
	
	while(1){__asm__("wfi\n\t");}   // WFI is similar to the HLT in x86_64

    return EFI_SUCCESS;
}
