#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_INPUT_KEY key;
    
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Hello from UEFI running on Windows!\n");
    Print(L"Press any key to exit...\n");
    uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
    while (uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) == EFI_NOT_READY);

    return EFI_SUCCESS;
}