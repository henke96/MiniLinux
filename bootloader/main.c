#include <Uefi.h>

EFI_STATUS efiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{
    EFI_STATUS status = systemTable->ConOut->OutputString(systemTable->ConOut, L"Hello World\r\n");
    if (EFI_ERROR(status)) return status;
    return EFI_SUCCESS;
}