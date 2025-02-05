// UEFI From Scratch for AARCH64 - ThatOSDev ( 2024 )
// https://github.com/ThatOSDev/EFI_AARCH64

#pragma once

#define EFI_SUCCESS               0x0000000000000000
#define EFI_ERROR                 0x8000000000000000 
#define EFI_LOAD_ERROR            (EFI_ERROR | 0x0000000000000001)
#define EFI_INVALID_PARAMETER     (EFI_ERROR | 0x0000000000000002)
#define EFI_UNSUPPORTED           (EFI_ERROR | 0x0000000000000003)
#define EFI_BAD_BUFFER_SIZE       (EFI_ERROR | 0x0000000000000004)
#define EFI_BUFFER_TOO_SMALL      (EFI_ERROR | 0x0000000000000005)
#define EFI_NOT_READY             (EFI_ERROR | 0x0000000000000006)
#define EFI_DEVICE_ERROR          (EFI_ERROR | 0x0000000000000007)
#define EFI_WRITE_PROTECTED       (EFI_ERROR | 0x0000000000000008)
#define EFI_OUT_OF_RESOURCES      (EFI_ERROR | 0x0000000000000009)
#define EFI_VOLUME_CORRUPTED      (EFI_ERROR | 0x000000000000000A)
#define EFI_VOLUME_FULL           (EFI_ERROR | 0x000000000000000B)
#define EFI_NO_MEDIA              (EFI_ERROR | 0x000000000000000C)
#define EFI_MEDIA_CHANGED         (EFI_ERROR | 0x000000000000000D)
#define EFI_NOT_FOUND             (EFI_ERROR | 0x000000000000000E)
#define EFI_ACCESS_DENIED         (EFI_ERROR | 0x000000000000000F)
#define EFI_NO_RESPONSE           (EFI_ERROR | 0x0000000000000010)
#define EFI_NO_MAPPING            (EFI_ERROR | 0x0000000000000011)
#define EFI_TIMEOUT               (EFI_ERROR | 0x0000000000000012)
#define EFI_NOT_STARTED           (EFI_ERROR | 0x0000000000000013)
#define EFI_ALREADY_STARTED       (EFI_ERROR | 0x0000000000000014)
#define EFI_ABORTED               (EFI_ERROR | 0x0000000000000015)
#define EFI_ICMP_ERROR            (EFI_ERROR | 0x0000000000000016)
#define EFI_TFTP_ERROR            (EFI_ERROR | 0x0000000000000017)
#define EFI_PROTOCOL_ERROR        (EFI_ERROR | 0x0000000000000018)
#define EFI_INCOMPATIBLE_VERSION  (EFI_ERROR | 0x0000000000000019)
#define EFI_SECURITY_VIOLATION    (EFI_ERROR | 0x000000000000001A)
#define EFI_CRC_ERROR             (EFI_ERROR | 0x000000000000001B)
#define EFI_END_OF_MEDIA          (EFI_ERROR | 0x000000000000001C)
#define EFI_END_OF_FILE           (EFI_ERROR | 0x000000000000001D)
#define EFI_INVALID_LANGUAGE      (EFI_ERROR | 0x000000000000001E)
#define EFI_COMPROMISED_DATA      (EFI_ERROR | 0x000000000000001F)
#define EFI_IP_ADDRESS_CONFLICT   (EFI_ERROR | 0x0000000000000020)
#define EFI_HTTP_ERROR            (EFI_ERROR | 0x0000000000000021)

#define EFI_WARN_UNKNOWN_GLYPH     0x0000000000000001
#define EFI_WARN_DELETE_FAILURE    0x0000000000000002
#define EFI_WARN_WRITE_FAILURE     0x0000000000000003
#define EFI_WARN_BUFFER_TOO_SMALL  0x0000000000000004
#define EFI_WARN_STALE_DATA        0x0000000000000005
#define EFI_WARN_FILE_SYSTEM       0x0000000000000006
#define EFI_WARN_RESET_REQUIRED    0x0000000000000007

// Adding this for convenience
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
