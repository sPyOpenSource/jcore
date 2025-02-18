// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://codeberg.org/ThatOSDev/EFI_AARCH64

#include "efi.h"
#include "ErrorCodes.h"
#include "efilibs.h"
#include "all.h"

/*#define EFI_FILE_INFO_ID \
  { \
    0x9576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b } \
  }*/

UINT64 getFileSize(EFI_FILE_PROTOCOL *FileHandle)
{
  UINT64* FileSize = 0;
  FileHandle->SetPosition(FileHandle, 0xFFFFFFFFFFFFFFFFULL);
  FileHandle->GetPosition(FileHandle, FileSize);
  FileHandle->SetPosition(FileHandle, 0);
  return *FileSize;
}

void loadIt(DomainDesc * domain, char *libname, char* codefilepos, int size, EFI_ALLOCATE_POOL AllocatePool);

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
	InitEFI(image, system);

    EFI_PHYSICAL_ADDRESS ExternalFileBuffer0 = 0;
    EFI_PHYSICAL_ADDRESS ExternalFileBuffer1 = 0;
    EFI_PHYSICAL_ADDRESS ExternalFileBuffer2 = 0;

    EFI_FILE_PROTOCOL* zero = openFile(u"zero.jll");
    EFI_FILE_PROTOCOL* jdk  = openFile(u"jdk.jll");
    EFI_FILE_PROTOCOL* init = openFile(u"init.jll");

    //EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;
    //EFI_FILE_INFO *FileInfo;
    EFI_ALLOCATE_POOL AllocatePool = SystemTable->BootServices->AllocatePool;
    EFI_STATUS Status;// = zero->GetInfo(zero, &gEfiFileInfoGuid, &FileInfoSize, NULL);

    UINT64 FileSize0 = getFileSize(init);
    UINT64 FileSize1 = getFileSize(zero);
    UINT64 FileSize2 = getFileSize(jdk);

    Status = AllocatePool(EfiLoaderCode, FileSize0, (void**)&ExternalFileBuffer0);
    Status = AllocatePool(EfiLoaderCode, FileSize1, (void**)&ExternalFileBuffer1);
    Status = AllocatePool(EfiLoaderCode, FileSize2, (void**)&ExternalFileBuffer2);

    //AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    //Status = zero->GetInfo(zero, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

	SetTextColor(EFI_BROWN);
    wprintf(u"AllocatePool ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));

    init->SetPosition(init, 0);
    init->Read(init, &FileSize0, (void *)ExternalFileBuffer0);

    zero->SetPosition(zero, 0);
    zero->Read(zero, &FileSize1, (void *)ExternalFileBuffer1);

    jdk->SetPosition(jdk, 0);
    jdk->Read(jdk,   &FileSize2, (void *)ExternalFileBuffer2);

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
    
	loadIt(NULL, "zero.jll", codefile, FileSize1, AllocatePool);
    codefile = (char*)ExternalFileBuffer0;
    loadIt(NULL, "init.jll", codefile, FileSize0, AllocatePool);
    codefile = (char*)ExternalFileBuffer2;
    loadIt(NULL, "jdk.jll",  codefile, FileSize2, AllocatePool);
	init->SetPosition(init, 0);
    
    SetTextColor(EFI_GREEN);
    SetTextPosition(10, 20);
    wprintf(u"Hit Any Key to execute testfile");

    HitAnyKey();
	
	SetGraphicsColor(BLACK);
	
	CreateFilledBox(150, 150, 800, 500);
	
	SetGraphicsColor(ORANGE);
	
	// Execute File, get return number --> 349587 \ 055593
	//int (*KernelBinFile)(void) = (int (*)(void)) ((UINT8 *)ExternalFileBuffer0);
    //int g = KernelBinFile();
    //KernelBinFile();
//__asm__("mov esp,INITSTACK");
//__asm__("mov 0,%edi");
//__asm__("mov 0,%rax");
//__asm__("call *%rax");
    SetTextColor(EFI_LIGHTMAGENTA);
    //wprintf(u"\r\nThe returned number is : %d", g);

    SetTextColor(EFI_GREEN);
    SetTextPosition(10, 13);
    wprintf(u"Hit Any Key to shutdown");

    HitAnyKey();
	
	SHUTDOWN();
	
    SetTextColor(EFI_GREEN);
    SetTextPosition(0, 22);
    wprintf(u"If you see this, something went wrong. Manually turn off the computer.");
	
	//while(1){__asm__("wfi\n\t");}   // WFI is similar to the HLT in x86_64
	while(1){__asm__("hlt\n\t");}   // WFI is similar to the HLT in x86_64

    return EFI_SUCCESS;
}
