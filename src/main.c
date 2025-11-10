#include <efi.h>
#include <efilib.h>

void DisplaySystemInfo(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- System Information ----\n");
    Print(L"\nFirmware Vendor: %s\n", SystemTable->FirmwareVendor);
    Print(L"Firmware Revision: %d\n", SystemTable->Hdr.Revision);
    Print(L"UEFI Version: %d.%02d\n\n", SystemTable->Hdr.Revision >> 16, SystemTable->Hdr.Revision & 0xFFFF);
}

void ChangeTextColor(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    SIMPLE_TEXT_OUTPUT_MODE *mode;

    if (SystemTable == NULL || SystemTable->ConOut == NULL) {
        Print(L"Console output not available\n");
        return;
    }

    mode = SystemTable->ConOut->Mode;
    if (mode == NULL) {
        Print(L"ConOut->Mode is NULL; cannot query modes\n");
    } else {
        Print(L"Current Mode: %d, MaxMode: %d\n", mode->Mode, mode->MaxMode);
    }

    INT32 want_mode = 0;
    if (mode != NULL && (want_mode < 0 || want_mode >= mode->MaxMode)) {
        Print(L"Requested mode %d out of range (0..%d)\n", want_mode, mode ? mode->MaxMode - 1 : -1);
    } else {
        Status = uefi_call_wrapper(SystemTable->ConOut->SetMode, 2, SystemTable->ConOut, want_mode);
        if (EFI_ERROR(Status)) {
            Print(L"SetMode(%d) failed: %r\n", want_mode, Status);
        } else {
            Print(L"SetMode(%d) OK\n", want_mode);
        }
    }

    UINTN attr = EFI_TEXT_ATTR(EFI_MAGENTA, EFI_BROWN);
    Status = uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2, SystemTable->ConOut, attr);
    if (EFI_ERROR(Status)) {
        Print(L"SetAttribute(0x%02x) failed: %r\n", attr, Status);
    } else {
        uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
        Print(L"Text color changed (attr=0x%02x)\n", attr);
    }
}

void DisplayCurrentTime(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_TIME Time;
    EFI_TIME_CAPABILITIES Capabilities;

    if (SystemTable == NULL || SystemTable->RuntimeServices == NULL) {
        Print(L"Runtime services not available\n");
        return;
    }

    Status = uefi_call_wrapper(SystemTable->RuntimeServices->GetTime, 2, &Time, &Capabilities);
    if (EFI_ERROR(Status)) {
        Print(L"GetTime failed: %r\n", Status);
        return;
    }

    Print(L"Current time: %02d-%02d-%04d %02d:%02d:%02d.%09d\n",
          Time.Day, Time.Month, Time.Year,
          Time.Hour, Time.Minute, Time.Second,
          Time.Nanosecond);

    Print(L"  TimeZone: %d  Daylight: 0x%x\n", Time.TimeZone, Time.Daylight);
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_INPUT_KEY Key;
    INTN Menu = 0;
    
    InitializeLib(ImageHandle, SystemTable);

    Print(L"Hello from UEFI running on Windows!\n");

    while (1)
    {   
        SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
        Print(L"\n----UEFI Menu----\n");
        Print(L"1. Display System Info\n");
        Print(L"2. Change Text Color\n");
        Print(L"3. Get Current Time\n");
        Print(L"4. Exit\n\n");
        Print(L"Select an option: \n");

        uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
        while (uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Key) == EFI_NOT_READY);

        Menu = Key.UnicodeChar - '0';

        switch (Menu) {
            case 1:
                DisplaySystemInfo(SystemTable);
                break;
            case 2:
                ChangeTextColor(SystemTable);
                break;
            case 3:
                DisplayCurrentTime(SystemTable);
                break;
            case 4:
                Print(L"Exiting UEFI application.\n");
                return EFI_SUCCESS;
            default:
                Print(L"Invalid option. Please try again.\n");
                break;
                
            }
            
        Print(L"\nPress any key to return to menu...\n");
        while (uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Key) == EFI_NOT_READY);
    }
    

    return EFI_SUCCESS;
}