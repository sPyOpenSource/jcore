// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://codeberg.org/ThatOSDev/EFI_AARCH64

#include "efi.h"
#include "ErrorCodes.h"
#include "efilibs.h"

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
	InitEFI(image, system);

    EFI_PHYSICAL_ADDRESS ExternalFileBuffer = 0;
    EFI_PHYSICAL_ADDRESS ExternalFileBuffer1 = 0;

    EFI_FILE_PROTOCOL* efimyfile = openFile(u"testfile.bin");
    EFI_FILE_PROTOCOL* zero = openFile(u"zero.jll");

    UINT64 fsize = 0x00001000;

    EFI_STATUS Status = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderCode, 1, &ExternalFileBuffer);
    EFI_STATUS Status1 = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderCode, 2, &ExternalFileBuffer1);

	SetTextColor(EFI_BROWN);
    wprintf(u"AllocatePool ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));

    efimyfile->SetPosition(efimyfile, 0);
    
    efimyfile->Read(efimyfile, &fsize, (void *)ExternalFileBuffer);

zero->SetPosition(zero, 0);
zero->Read(zero, &fsize, (void *)ExternalFileBuffer1);

    SetTextColor(EFI_GREEN);
    wprintf(u"\r\nRead ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));

    SetTextColor(EFI_LIGHTCYAN);
    wprintf(u"\r\nFirst 5 Bytes\r\n");
    SetTextColor(EFI_LIGHTRED);
    UINT8* test = (UINT8*)ExternalFileBuffer1;

    for(int m = 0; m < 5; m++)
    {
        int j = *test;
        wprintf(u"%x ", j);
        test++;
    }
	
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
