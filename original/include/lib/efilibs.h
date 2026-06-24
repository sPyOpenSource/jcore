// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://codeberg.org/ThatOSDev/EFI_AARCH64

#ifndef EFILIBS_H
#define EFILIBS_H

#include "clib.h"

#define HEX 16

#define MAX_LENGTH 22

             // ARGB             
#define ORANGE 0xffffa500
#define CYAN   0xff00ffff
#define RED    0xffff0000
#define GREEN  0xff00ff00
#define BLUE   0xff0000ff
#define GRAY   0xff888888
#define WHITE  0xffffffff
#define BLACK  0xff000000

typedef struct {
	uint32_t cols;  // Left / Right
	uint32_t rows;  // Up / Down
} Timer_Context;

// From the GNU-EFI
int64_t RtCompareGuid(EFI_GUID* Guid1, EFI_GUID* Guid2);

int64_t CompareGuid(EFI_GUID* Guid1, EFI_GUID* Guid2);

void SetTextPosition(UINT32 Col, UINT32 Row);

void ResetScreen(void);

void ClearScreen(void);

void printUInt64Digits(uint64_t num, uint64_t base);

void printIntDigits(int32_t num);

void wprintf(CHAR16* txt, ...);

void HitAnyKey(void);

void ResetKeyboard(void);

BOOLEAN GetKey(CHAR16 key);

EFI_STATUS CheckKey(void);

void Delay(uint64_t d);

void SetTextColor(uint64_t color);

void COLD_REBOOT(void);

void WARM_REBOOT(void);

void SHUTDOWN(void);

// This sets the color of the pixels ( Graphics Color )
void SetGraphicsColor(UINT32 color);

// This positions the pixel in the row and column ( X and Y )
void SetPixel(UINT32 xPos, UINT32 yPos);

// This creates a filled box of pixels.
// NOTE : Call SetGraphicsColor prior to using this function.
void CreateFilledBox(UINT32 xPos, UINT32 yPos, UINT32 w, UINT32 h);

void InitializeGOP(void);

void InitializeFILESYSTEM(void);

EFI_FILE_PROTOCOL* openFile(CHAR16* FileName);

void closeFile(EFI_FILE_PROTOCOL* FileHandle);

void readFile(CHAR16* FileName);

void InitEFI(EFI_HANDLE handle, EFI_SYSTEM_TABLE  *table);

#endif // EFILIBS_H
