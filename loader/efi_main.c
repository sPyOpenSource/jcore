// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://codeberg.org/ThatOSDev/EFI_AARCH64

//#include "efi.h"
//#include "ErrorCodes.h"
//#include "efilibs.h"
#include "all.h"

/*#define EFI_FILE_INFO_ID \
  { \
    0x9576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b } \
  }*/
// The GUID to set the correct Protocol.
struct EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID    = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
struct EFI_GUID EFI_FILE_INFO_GUID                   = {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
struct EFI_GUID EFI_LOADED_IMAGE_PROTOCOL_GUID       = {0x5b1b31a1,  0x9562, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
struct EFI_GUID EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID = {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
struct EFI_GUID EFI_DEVICE_PATH_PROTOCOL_GUID        = {0x09576e91,  0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

EFI_HANDLE                       ImageHandle;
EFI_SYSTEM_TABLE                *SystemTable;
EFI_STATUS                       ERROR_STATUS;
EFI_GRAPHICS_OUTPUT_PROTOCOL    *gop;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
EFI_FILE_PROTOCOL               *RootFS;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL    GraphicsColor;  // Public Variable for now.

uint32_t DisplayWidth  = 0;
uint32_t DisplayHeight = 0;
void* OSBuffer_Handle;
EFI_EVENT timer_event;
EFI_INPUT_KEY CheckKeystroke;

UINT64 getFileSize(EFI_FILE_PROTOCOL *FileHandle)
{
  UINT64* FileSize = 0;
  FileHandle->SetPosition(FileHandle, 0xFFFFFFFFFFFFFFFFULL);
  FileHandle->GetPosition(FileHandle, FileSize);
  FileHandle->SetPosition(FileHandle, 0);
  return *FileSize;
}

void loadIt(DomainDesc * domain, char *libname, char* codefilepos, int size);
DomainDesc * domainZero;

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

    UINT64 FileSize0 = getFileSize(zero);
    UINT64 FileSize1 = getFileSize(jdk);
    UINT64 FileSize2 = getFileSize(init);

    Status = AllocatePool(EfiLoaderCode, FileSize0, (void**)&ExternalFileBuffer0);
    Status = AllocatePool(EfiLoaderCode, FileSize1, (void**)&ExternalFileBuffer1);
    Status = AllocatePool(EfiLoaderCode, FileSize2, (void**)&ExternalFileBuffer2);

    //AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    //Status = zero->GetInfo(zero, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

	SetTextColor(EFI_BROWN);
    wprintf(u"AllocatePool ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));
    AllocatePool(EfiLoaderData, sizeof(DomainDesc), (void**)&domainZero);
    domainZero->numberOfLibs = 0;
    domainZero->maxNumberOfLibs = 100;
    AllocatePool(EfiLoaderData, sizeof(LibDesc) * domainZero->maxNumberOfLibs, (void**)&domainZero->libs);
    zero->SetPosition(zero, 0);
    zero->Read(zero, &FileSize0, (void *)ExternalFileBuffer0);
    char* codefile = (char*)ExternalFileBuffer0;
    loadIt(domainZero, "zero.jll", codefile, FileSize0);

    jdk->SetPosition(jdk, 0);
    jdk->Read(jdk, &FileSize1, (void *)ExternalFileBuffer1);
    codefile = (char*)ExternalFileBuffer1;
    loadIt(domainZero, "jdk.jll", codefile, FileSize1);

    init->SetPosition(init, 0);
    init->Read(init, &FileSize2, (void *)ExternalFileBuffer2);

    SetTextColor(EFI_GREEN);
    wprintf(u"Read ExternalFileBuffer");
	SetTextColor(EFI_LIGHTCYAN);
    wprintf(CheckStandardEFIError(Status));

    SetTextColor(EFI_LIGHTCYAN);
    //wprintf(u"\r\nFirst 5 Bytes\r\n");
    SetTextColor(EFI_LIGHTRED);

    /*for(int m = 0; m < 5; m++)
    {
        int j = *codefile;
        wprintf(u"%x ", j);
        codefile++;
    }*/
    
    codefile = (char*)ExternalFileBuffer2;
    loadIt(domainZero, "init.jll", codefile, FileSize2);
    
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

unsigned short int* CheckStandardEFIError(unsigned long long s)
{
    switch(s)
    {
        case EFI_LOAD_ERROR:
        {
            return (unsigned short int*)L" Load Error\r\n";
        }
        case EFI_INVALID_PARAMETER:
        {
            return (unsigned short int*)L" Invalid Parameter\r\n";
        }
        case EFI_UNSUPPORTED:
        {
            return (unsigned short int*)L" Unsupported\r\n";
        }
        case EFI_BAD_BUFFER_SIZE:
        {
            return (unsigned short int*)L" Bad Buffer Size\r\n";
        }
        case EFI_BUFFER_TOO_SMALL:
        {
            return (unsigned short int*)L" Buffer Too Small\r\n";
        }
        case EFI_NOT_READY:
        {
            return (unsigned short int*)L" Not Ready\r\n";
        }
        case EFI_DEVICE_ERROR:
        {
            return (unsigned short int*)L" Device Error\r\n";
        }
        case EFI_WRITE_PROTECTED:
        {
            return (unsigned short int*)L" Write Protected\r\n";
        }
        case EFI_OUT_OF_RESOURCES:
        {
            return (unsigned short int*)L" Out Of Resources\r\n";
        }
        case EFI_VOLUME_CORRUPTED:
        {
            return (unsigned short int*)L" Volume Corrupted\r\n";
        }
        case EFI_VOLUME_FULL:
        {
            return (unsigned short int*)L" Volume Full\r\n";
        }
        case EFI_NO_MEDIA:
        {
            return (unsigned short int*)L" No Media\r\n";
        }
        case EFI_MEDIA_CHANGED:
        {
            return (unsigned short int*)L" Media Changed\r\n";
        }
        case EFI_NOT_FOUND:
        {
            return (unsigned short int*)L" File Not Found\r\n";
        }
        case EFI_ACCESS_DENIED:
        {
            return (unsigned short int*)L" Access Denied\r\n";
        }
        case EFI_NO_RESPONSE:
        {
            return (unsigned short int*)L" No Response\r\n";
        }
        case EFI_NO_MAPPING:
        {
            return (unsigned short int*)L" No Mapping\r\n";
        }
        case EFI_TIMEOUT:
        {
            return (unsigned short int*)L" Timeout\r\n";
        }
        case EFI_NOT_STARTED:
        {
            return (unsigned short int*)L" Not Started\r\n";
        }
        case EFI_ALREADY_STARTED:
        {
            return (unsigned short int*)L" Already Started\r\n";
        }
        case EFI_ABORTED:
        {
            return (unsigned short int*)L" Aborted\r\n";
        }
        case EFI_ICMP_ERROR:
        {
            return (unsigned short int*)L" ICMP Error\r\n";
        }
        case EFI_TFTP_ERROR:
        {
            return (unsigned short int*)L" TFTP Error\r\n";
        }
        case EFI_PROTOCOL_ERROR:
        {
            return (unsigned short int*)L" Protocol Error\r\n";
        }
        case EFI_INCOMPATIBLE_VERSION:
        {
            return (unsigned short int*)L" Incompatible Version\r\n";
        }
        case EFI_SECURITY_VIOLATION:
        {
            return (unsigned short int*)L" Security Violation\r\n";
        }
        case EFI_CRC_ERROR:
        {
            return (unsigned short int*)L" CRC Error\r\n";
        }
        case EFI_END_OF_MEDIA:
        {
            return (unsigned short int*)L" End Of Media\r\n";
        }
        case EFI_END_OF_FILE:
        {
            return (unsigned short int*)L" End Of File\r\n";
        }
        case EFI_INVALID_LANGUAGE:
        {
            return (unsigned short int*)L" Invalid Language\r\n";
        }
        case EFI_COMPROMISED_DATA:
        {
            return (unsigned short int*)L" Compromised Data\r\n";
        }
        case EFI_IP_ADDRESS_CONFLICT:
        {
            return (unsigned short int*)L" IP Address Conflict\r\n";
        }
        case EFI_HTTP_ERROR:
        {
            return (unsigned short int*)L" End Of File\r\n";
        }
        case EFI_WARN_UNKNOWN_GLYPH:
        {
            return (unsigned short int*)L" WARNING - Unknown Glyph\r\n";
        }
        case EFI_WARN_DELETE_FAILURE:
        {
            return (unsigned short int*)L" WARNING - Delete Failure\r\n";
        }
        case EFI_WARN_WRITE_FAILURE:
        {
            return (unsigned short int*)L" WARNING - Write Failure\r\n";
        }
        case EFI_WARN_BUFFER_TOO_SMALL:
        {
            return (unsigned short int*)L" WARNING - Buffer Too Small\r\n";
        }
        case EFI_WARN_STALE_DATA:
        {
            return (unsigned short int*)L" WARNING - Stale Data\r\n";
        }
        case EFI_WARN_FILE_SYSTEM:
        {
            return (unsigned short int*)L" WARNING - File System\r\n";
        }
        case EFI_WARN_RESET_REQUIRED:
        {
            return (unsigned short int*)L" WARNING - Reset Required\r\n";
        }
        case EFI_SUCCESS:
        {
            return (unsigned short int*)L" Successful\r\n";
        }
    }
    return (unsigned short int*)L" ERROR\r\n";
}

size_t strlen(const char* str)
{
	const char* strCount = str;

	while (*strCount++);
	return strCount - str - 1;
}

int strcmp(const char* a, const char* b)
{
	size_t length = strlen(a);
	for(size_t i = 0; i < length; i++)
	{
		if(a[i] < b[i]){return -1;}
		if(a[i] > b[i]){return 1;}
	}
	return 0;
}

void itoa(uint64_t n, uint64_t* buffer, uint32_t basenumber)
{
	unsigned long int hold;
	int i, j;
	hold = n;
	i = 0;
	
	do{
		hold = n % basenumber;
		buffer[i++] = (hold < 10) ? (hold + '0') : (hold + 'a' - 10);
	} while(n /= basenumber);
	buffer[i--] = 0;
	
	for(j = 0; j < i; j++, i--)
	{
		hold = buffer[j];
		buffer[j] = buffer[i];
		buffer[i] = hold;
	}
}

void* memcpy(void* destination, const void* source, size_t num)
{
    for (size_t i = 0; i < num; i++)
        ((uint8_t*) destination)[i] = ((uint8_t*) source)[i];
    return destination;
}

// GCC Version
void *memset (void *dest, register int val, register size_t len)
{
  register unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;

  return dest;
}

int64_t RtCompareGuid(EFI_GUID* Guid1, EFI_GUID* Guid2)
{
    int32_t *g1, *g2, r;
    g1 = (int32_t *) Guid1;
    g2 = (int32_t *) Guid2;
    r  = g1[0] - g2[0];
    r |= g1[1] - g2[1];
    r |= g1[2] - g2[2];
    r |= g1[3] - g2[3];
    return r;
}

int64_t CompareGuid(EFI_GUID* Guid1, EFI_GUID* Guid2)
{
    return RtCompareGuid (Guid1, Guid2);
}

void SetTextPosition(UINT32 Col, UINT32 Row)
{
    // Sets the Column and Row of the text screen cursor position.
    SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, Col, Row);
}

void ResetScreen(void)
{
    // This resets the whole console ( A.K.A. your display screen ) interface.
    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
}

void ClearScreen(void)
{
    // This clears the screen buffer, but does not reset it.
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

void printUInt64Digits(uint64_t num, uint64_t base)
{
    CHAR16 uint64Str[MAX_LENGTH] = {'\0'};
	
	const CHAR16* digits = u"0123456789ABCDEF";
	uint64_t i = 0;

	do
	{
		uint64Str[i++] = digits[num % base];
        num /= base;
	} while(num > 0);
	
    uint64Str[i--] = '\0';
	
	for(uint64_t j = 0; j < i; j++, i--)
	{
		CHAR16 temp = uint64Str[i];
		uint64Str[i]  = uint64Str[j];
		uint64Str[j]  = temp;
	}
	
	SystemTable->ConOut->OutputString(SystemTable->ConOut, uint64Str);
}

void printIntDigits(int32_t num)
{
    CHAR16 int32Str[12] = {'\0'};
	
	const CHAR16* digits = u"0123456789";
	uint64_t i = 0;
	int negative = (num < 0);
	if(negative) {num = -num;}
	
	do
	{
		int32Str[i++] = digits[num % 10];
		num /= 10;
	} while(num > 0);
	
	if(negative) {int32Str[i++] = '-';}
	
	int32Str[i--] = '\0';
	
	for(uint64_t j = 0; j < i; j++, i--)
	{
		CHAR16 temp = int32Str[i];
		int32Str[i] = int32Str[j];
		int32Str[j] = temp;
	}
	
	SystemTable->ConOut->OutputString(SystemTable->ConOut, int32Str);
}

void wprintf(CHAR16* txt, ...)
{
	CHAR16 charStr[2]   = {'\0'};

	va_list args;
	va_start(args, txt);
	
	for(uint64_t i = 0; txt[i] != '\0'; i++)
	{
		if(txt[i] == '%')
		{
			i++;
			switch(txt[i])
			{
                case 'c': {
                    charStr[0] = (CHAR16)va_arg(args, int);
                    SystemTable->ConOut->OutputString(SystemTable->ConOut, charStr);
					break;
                }
				case 's':
				{
					uint16_t* varString = va_arg(args, uint16_t*);
					SystemTable->ConOut->OutputString(SystemTable->ConOut, varString);
					break;
				}
				case 'd':
				{
					int32_t number = va_arg(args, int32_t);
					printIntDigits(number);
					break;
				}
                case 'b':
				{
					uint64_t bin = va_arg(args, uint64_t);
					printUInt64Digits(bin, 2);
					break;
                }
				case 'x':
				{
					uint64_t hex = va_arg(args, uint64_t);
					printUInt64Digits(hex, 16);
					break;
				}
				case 'l':
				{
					if(txt[i+1] == 'l' && txt[i+2] == 'u')
					{
						i+=2;
                        uint64_t num = va_arg(args, uint64_t);
						printUInt64Digits(num, 10);
					}
					break;
				}

				default:
				{
					SystemTable->ConOut->SetAttribute(SystemTable->ConOut, EFI_RED);
				    SystemTable->ConOut->OutputString(SystemTable->ConOut, u"\r\nERROR : Invalid format: %");
					SystemTable->ConOut->OutputString(SystemTable->ConOut, &txt[i]);
					SystemTable->ConOut->OutputString(SystemTable->ConOut, u"\r\n");
					return;
					break;
				}
			}
		} else {
			charStr[0] = txt[i];
			charStr[1] = '\0';
			SystemTable->ConOut->OutputString(SystemTable->ConOut, charStr);
		}
	}

	va_end(args);
}

void HitAnyKey(void)
{
    // This clears the keyboard buffer.
    SystemTable->ConIn->Reset(SystemTable->ConIn, 1);

    // We setup the struct to take keyboard input.
    EFI_INPUT_KEY Key;

    // In a while loop to see if the keyboard has a key stroke in the buffer.
	// Added the __asm__("wfi\n\t"); code to slow the CPU down.
    //while((SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY) {__asm__("wfi\n\t");};
    while((SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY) {__asm__("hlt\n\t");};
}

void ResetKeyboard(void)
{
    SystemTable->ConIn->Reset(SystemTable->ConIn, 1);
}

//EFI_INPUT_KEY CheckKeystroke;
BOOLEAN GetKey(CHAR16 key)
{
    if(CheckKeystroke.UnicodeChar == key)
    {
        return 1;
    } else {
        return 0;
    }
}

EFI_STATUS CheckKey(void)
{
    return SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &CheckKeystroke);
}

void Delay(uint64_t d)
{
    // The Stall function is set as microseconds.
    SystemTable->BootServices->Stall(d);
}

void SetTextColor(uint64_t color)
{
	SystemTable->ConOut->SetAttribute(SystemTable->ConOut, color);
}

void COLD_REBOOT(void)
{
    // Hardware Reboot
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, 0);
}

void WARM_REBOOT(void)
{
    // Software reboot
    SystemTable->RuntimeServices->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, 0);
}

void SHUTDOWN(void)
{
    // Shuts off the computer
    // NOTE : This does not work in VirtualBox.
    // WORKS in QEMU !!!
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
}

// This sets the color of the pixels ( Graphics Color )
void SetGraphicsColor(UINT32 color)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL GColor;
    GColor.Reserved = color >> 24;
    GColor.Red      = color >> 16;
    GColor.Green    = color >> 8;
    GColor.Blue     = color;
    GraphicsColor = GColor;
}

// This positions the pixel in the row and column ( X and Y )
void SetPixel(UINT32 xPos, UINT32 yPos)
{
    // TODO : Add in a choice instead of defaulting to EfiBltVideoFill.
    gop->Blt(gop, &GraphicsColor, EfiBltVideoFill, 0, 0, xPos, yPos, 1, 1, 0);
}

// This creates a filled box of pixels.
// NOTE : Call SetGraphicsColor prior to using this function.
void CreateFilledBox(UINT32 xPos, UINT32 yPos, UINT32 w, UINT32 h)
{
    // TODO : Add in a choice instead of defaulting to EfiBltVideoFill.
    gop->Blt(gop, &GraphicsColor, EfiBltVideoFill, 0, 0, xPos, yPos, w, h, 0);
}

void InitializeGOP(void)
{
    SetTextColor(EFI_BROWN);
    wprintf(L"\r\nLoading Graphics Output Protocol ... ");
    EFI_STATUS Status = SystemTable->BootServices->LocateProtocol(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, 0, (void**)&gop);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
	
	DisplayWidth  = gop->Mode->Info->HorizontalResolution;
	DisplayHeight = gop->Mode->Info->VerticalResolution;
}

void InitializeFILESYSTEM(void)
{
    EFI_STATUS Status;
    // To load a file, you must have a file system. EFI takes advantage of the FAT32 file system.
    SetTextColor(EFI_BROWN);
    wprintf(L"LoadedImage ... ");
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_LOADED_IMAGE_PROTOCOL_GUID, (void**)&LoadedImage);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
    
    SetTextColor(EFI_BROWN);
    wprintf(L"DevicePath ... ");
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    Status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_DEVICE_PATH_PROTOCOL_GUID, (void**)&DevicePath);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
    
    SetTextColor(EFI_BROWN);
    wprintf(L"Volume ... ");
    Status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void**)&Volume);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
}

EFI_FILE_PROTOCOL* openFile(CHAR16* FileName)
{
    // This opens a file from the EFI FAT32 file system volume.
    // It loads from root, so you must supply full path if the file is not in the root.
    // Example : "somefolder//myfile"  <--- Notice the double forward slash.
    EFI_STATUS Status;
    SetTextColor(EFI_BROWN);
    wprintf(L"RootFS ... ");
    EFI_FILE_PROTOCOL* RootFS;
    Status = Volume->OpenVolume(Volume, &RootFS);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
    
    SetTextColor(EFI_BROWN);
    wprintf(L"Opening File ... ");
    EFI_FILE_PROTOCOL* FileHandle = NULL;
    Status = RootFS->Open(RootFS, &FileHandle, FileName, 0x0000000000000001, 0);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
    
    return FileHandle;
}

void closeFile(EFI_FILE_PROTOCOL* FileHandle)
{
    // This closes the file.
    EFI_STATUS Status;
    SetTextColor(EFI_BROWN);
    wprintf(L"Closing File ... ");
    Status = FileHandle->Close(FileHandle);
    SetTextColor(EFI_CYAN);
    wprintf(CheckStandardEFIError(Status));
}

void readFile(CHAR16* FileName)
{
	EFI_STATUS Status = 0;
    EFI_FILE_PROTOCOL* mytextfile = openFile(FileName);
    if(mytextfile != NULL)
    {
        SetTextColor(EFI_BROWN);
        wprintf(L"AllocatingPool ... ");
        Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, 0x00001000, (void**)&OSBuffer_Handle);
        SetTextColor(EFI_CYAN);
        wprintf(CheckStandardEFIError(Status));
    
        UINT64 fileSize = 0x00001000;
        
        SetTextColor(EFI_BROWN);
        wprintf(L"Reading File ... ");
        Status = mytextfile->Read(mytextfile, &fileSize, OSBuffer_Handle);
        SetTextColor(EFI_CYAN);
        wprintf(CheckStandardEFIError(Status));

        closeFile(mytextfile);
    }
}

void InitEFI(EFI_HANDLE handle, EFI_SYSTEM_TABLE  *table)
{
	ImageHandle   = handle;
	SystemTable   = table;
	ERROR_STATUS  = 0;
	ResetScreen();

    SetTextColor(EFI_WHITE);
    wprintf(u"EFI loaded on Hardware !\r\n");
    
    SetTextColor(EFI_GREEN);
    //wprintf(u"Hit Any Key to see Graphics and setup the FileSystem.");

    //HitAnyKey();
	
	//SetTextPosition(0, 5);

	InitializeFILESYSTEM();

	InitializeGOP();
	
	//SetGraphicsColor(ORANGE);
	//CreateFilledBox(50, 50, 100, 200);
	//SetGraphicsColor(RED);
	//CreateFilledBox(60, 60, 80, 30);
	SetTextPosition(0, 8);
	SetTextColor(EFI_YELLOW);
	wprintf(u"We have Graphics !!");

	SetTextColor(EFI_GREEN);
	
	SetTextPosition(0, 11);
}
