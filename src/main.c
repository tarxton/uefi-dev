#include <efi.h>
#include <efilib.h>

void DisplaySystemInfo(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- Extended System Information ----\n");
    Print(L"\nFirmware Vendor: %s\n", SystemTable->FirmwareVendor);
    Print(L"Firmware Revision: 0x%08x\n", SystemTable->Hdr.Revision);
    Print(L"UEFI Version: %d.%02d\n", SystemTable->Hdr.Revision >> 16, SystemTable->Hdr.Revision & 0xFFFF);
    
    Print(L"\n--- Console Information ---\n");
    if (SystemTable->ConOut && SystemTable->ConOut->Mode) {
        Print(L"Current Console Mode: %d\n", SystemTable->ConOut->Mode->Mode);
        Print(L"Available Console Modes: %d\n", SystemTable->ConOut->Mode->MaxMode);
    }
    
    Print(L"\n--- Table Headers ---\n");
    Print(L"Boot Services Table Revision: 0x%08x\n", SystemTable->BootServices->Hdr.Revision);
    Print(L"Runtime Services Table Revision: 0x%08x\n", SystemTable->RuntimeServices->Hdr.Revision);
}

void DisplayGraphicsInfo(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- Console Graphics Modes ----\n");
    
    SIMPLE_TEXT_OUTPUT_MODE *mode = SystemTable->ConOut->Mode;
    if (mode == NULL) {
        Print(L"Cannot query graphics modes\n");
        return;
    }
    
    Print(L"Current Mode: %d\n", mode->Mode);
    Print(L"Total Modes Available: %d\n\n", mode->MaxMode);
    
    UINTN columns = 0, rows = 0;
    for (INT32 i = 0; i < mode->MaxMode; i++) {
        if (uefi_call_wrapper(SystemTable->ConOut->QueryMode, 4, SystemTable->ConOut, i, &columns, &rows) == EFI_SUCCESS) {
            Print(L"Mode %d: %d x %d\n", i, columns, rows);
        }
    }
}

void DisplayBootInfo(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- Boot Information ----\n");
    
    UINTN dataSize = 0;
    EFI_STATUS Status;
    
    Print(L"Boot Services: ");
    if (SystemTable->BootServices) {
        Print(L"Available\n");
    } else {
        Print(L"Not Available\n");
    }
    
    Print(L"Runtime Services: ");
    if (SystemTable->RuntimeServices) {
        Print(L"Available\n");
    } else {
        Print(L"Not Available\n");
    }
    
    Print(L"\nNumber of Configuration Tables: %d\n", SystemTable->NumberOfTableEntries);
    
    Print(L"\n--- Configuration Tables ---\n");
    for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *Table = &SystemTable->ConfigurationTable[i];
        Print(L"Table %d GUID: %08x-%04x-%04x-", i,
              Table->VendorGuid.Data1,
              Table->VendorGuid.Data2,
              Table->VendorGuid.Data3);
        for (int j = 0; j < 8; j++) {
            Print(L"%02x", Table->VendorGuid.Data4[j]);
        }
        Print(L"\n");
    }
}

void DisplayMemoryInfo(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- Memory Information ----\n");
    
    EFI_STATUS Status;
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MemoryMapSize = 0;
    
    // Get memory map size
    Status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5, 
                              &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    
    Print(L"Total Memory Map Size: %d bytes\n", MemoryMapSize);
    Print(L"Memory Descriptor Size: %d bytes\n", DescriptorSize);
    Print(L"Memory Descriptor Version: %d\n", DescriptorVersion);
    Print(L"Number of Memory Descriptors: %d\n", MemoryMapSize / DescriptorSize);
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

    Print(L"\n---- Current System Time ----\n\n");
    Print(L"Date: %02d-%02d-%04d\n",
          Time.Day, Time.Month, Time.Year);
    Print(L"Time: %02d:%02d:%02d.%09d\n",
          Time.Hour, Time.Minute, Time.Second,
          Time.Nanosecond);

    Print(L"\nTimeZone: %d minutes\n", Time.TimeZone);
    Print(L"Daylight Saving: 0x%x\n", Time.Daylight);
    Print(L"Accuracy: %d ppm\n", Capabilities.Accuracy);
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
        Print(L"\n--- Text Color Changed ---\n");
        Print(L"Attribute: 0x%02x (Magenta on Brown)\n", attr);
    }
}

void DisplayUEFIVariables(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- UEFI Variables ----\n\n");
    
    EFI_STATUS Status;
    UINTN DataSize = 0;
    UINT32 Attributes = 0;
    UINT16 BootOrder[20];
    UINT16 BootCurrent = 0;
    
    // Get BootCurrent
    DataSize = sizeof(UINT16);
    Status = uefi_call_wrapper(SystemTable->RuntimeServices->GetVariable, 5,
                              L"BootCurrent", &gEfiGlobalVariableGuid, 
                              &Attributes, &DataSize, &BootCurrent);
    if (!EFI_ERROR(Status)) {
        Print(L"BootCurrent: Boot%04X\n", BootCurrent);
    } else {
        Print(L"BootCurrent: Not available\n");
    }
    
    // Get BootOrder
    DataSize = sizeof(BootOrder);
    Status = uefi_call_wrapper(SystemTable->RuntimeServices->GetVariable, 5,
                              L"BootOrder", &gEfiGlobalVariableGuid,
                              &Attributes, &DataSize, BootOrder);
    if (!EFI_ERROR(Status)) {
        Print(L"\nBootOrder (%d entries):\n", DataSize / sizeof(UINT16));
        for (UINTN i = 0; i < DataSize / sizeof(UINT16); i++) {
            Print(L"  [%d] Boot%04X\n", i, BootOrder[i]);
        }
    } else {
        Print(L"BootOrder: Not available\n");
    }
    
    // Get ConOut
    CHAR16 ConOut[256];
    DataSize = sizeof(ConOut);
    Status = uefi_call_wrapper(SystemTable->RuntimeServices->GetVariable, 5,
                              L"ConOut", &gEfiGlobalVariableGuid,
                              &Attributes, &DataSize, ConOut);
    if (!EFI_ERROR(Status)) {
        Print(L"\nConsole Output: Device connected\n");
    }
    
    // Get ConIn
    CHAR16 ConIn[256];
    DataSize = sizeof(ConIn);
    Status = uefi_call_wrapper(SystemTable->RuntimeServices->GetVariable, 5,
                              L"ConIn", &gEfiGlobalVariableGuid,
                              &Attributes, &DataSize, ConIn);
    if (!EFI_ERROR(Status)) {
        Print(L"Console Input: Device connected\n");
    }
    
    Print(L"\nVariable Attributes:\n");
    Print(L"  NV (Non-Volatile): %s\n", (Attributes & EFI_VARIABLE_NON_VOLATILE) ? L"Yes" : L"No");
    Print(L"  BS (Boot Services): %s\n", (Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS) ? L"Yes" : L"No");
    Print(L"  RT (Runtime): %s\n", (Attributes & EFI_VARIABLE_RUNTIME_ACCESS) ? L"Yes" : L"No");
}

void DisplayFileSystemInfo(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- File System Information ----\n\n");
    
    EFI_STATUS Status;
    EFI_HANDLE *Handles = NULL;
    UINTN HandleCount = 0;
    
    // Locate all Simple File System Protocol handles
    Status = uefi_call_wrapper(SystemTable->BootServices->LocateHandleBuffer, 5,
                              ByProtocol, &gEfiSimpleFileSystemProtocolGuid,
                              NULL, &HandleCount, &Handles);
    
    if (EFI_ERROR(Status)) {
        Print(L"No file systems found or error occurred\n");
        Print(L"Status: %r\n", Status);
        return;
    }
    
    Print(L"File Systems Found: %d\n\n", HandleCount);
    
    for (UINTN i = 0; i < HandleCount; i++) {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
        EFI_FILE_PROTOCOL *Root = NULL;
        EFI_FILE_PROTOCOL *File = NULL;
        EFI_FILE_INFO *FileInfo = NULL;
        UINTN FileInfoSize = 0;
        
        // Get the Simple File System Protocol
        Status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3,
                                  Handles[i], &gEfiSimpleFileSystemProtocolGuid,
                                  (VOID**)&FileSystem);
        
        if (!EFI_ERROR(Status) && FileSystem) {
            Print(L"File System %d:\n", i + 1);
            
            // Open the root directory
            Status = uefi_call_wrapper(FileSystem->OpenVolume, 2, FileSystem, &Root);
            
            if (!EFI_ERROR(Status) && Root) {
                Print(L"  Root Volume: Accessible\n");
                
                // Try to read first few entries
                CHAR16 FileName[256];
                UINTN Index = 0;
                UINTN MaxEntries = 5;
                
                Print(L"  First entries:\n");
                
                while (Index < MaxEntries) {
                    FileInfoSize = 0;
                    Status = uefi_call_wrapper(Root->Read, 3, Root, &FileInfoSize, NULL);
                    
                    if (FileInfoSize > 0) {
                        FileInfo = AllocatePool(FileInfoSize);
                        Status = uefi_call_wrapper(Root->Read, 3, Root, &FileInfoSize, FileInfo);
                        
                        if (!EFI_ERROR(Status) && FileInfo) {
                            if (FileInfo->FileName[0] == L'\0') {
                                break;
                            }
                            Print(L"    - %s %s\n", 
                                  (FileInfo->Attribute & EFI_FILE_DIRECTORY) ? L"[DIR]" : L"[FILE]",
                                  FileInfo->FileName);
                            FreePool(FileInfo);
                            Index++;
                        }
                    } else {
                        break;
                    }
                }
                
                uefi_call_wrapper(Root->Close, 1, Root);
            } else {
                Print(L"  Root Volume: Not accessible\n");
            }
            Print(L"\n");
        }
    }
    
    if (Handles) {
        FreePool(Handles);
    }
}

void DisplayHelp(EFI_SYSTEM_TABLE *SystemTable) {
    Print(L"\n---- UEFI Application Help ----\n\n");
    Print(L"This is a custom UEFI application demonstrating various\n");
    Print(L"system information and firmware capabilities.\n\n");
    Print(L"Menu Navigation:\n");
    Print(L"- Use UP/DOWN arrow keys to navigate menu\n");
    Print(L"- Press ENTER to select an option\n\n");
    Print(L"Available Options:\n");
    Print(L"1. System Info: Firmware and table information\n");
    Print(L"2. Graphics: Available console modes\n");
    Print(L"3. Boot Info: Boot and config table details\n");
    Print(L"4. Memory Info: Memory map information\n");
    Print(L"5. Current Time: System date and time\n");
    Print(L"6. File Systems: Mounted volumes and entries\n");
    Print(L"7. UEFI Variables: Boot variables and settings\n");
    Print(L"8. Text Color: Console colors\n");
    Print(L"9. Help: This help message\n");
    Print(L"10. Exit: Terminates the application\n\n");
    Print(L"Project Information:\n");
    Print(L"This application demonstrates UEFI boot services,\n");
    Print(L"runtime services, and system table access.\n\n");
    Print(L"Made for CS307 Operating Systems IUS.\n");
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_INPUT_KEY Key;
    INTN selectedOption = 1;
    INTN menuItems = 10;
    
    InitializeLib(ImageHandle, SystemTable);

    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    Print(L"\n====== UEFI System Information Tool ======\n");
    Print(L"Use UP/DOWN arrows to select, ENTER to confirm\n\n");

    // Print menu items once
    Print(L"> System Info\n");
    Print(L"  Graphics Modes\n");
    Print(L"  Boot Information\n");
    Print(L"  Memory Information\n");
    Print(L"  Current Time\n");
    Print(L"  File Systems\n");
    Print(L"  UEFI Variables\n");
    Print(L"  Text Color\n");
    Print(L"  Help\n");
    Print(L"  Exit\n");

    while (1)
    {
        uefi_call_wrapper(SystemTable->ConIn->Reset, 2, SystemTable->ConIn, FALSE);
        while (uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Key) == EFI_NOT_READY);

        // Handle arrow keys
        if (Key.ScanCode == SCAN_UP) {
            // Clear old selector at current position
            uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, 0, selectedOption + 3);
            Print(L"  ");
            
            selectedOption--;
            if (selectedOption < 1) {
                selectedOption = menuItems;
            }
            
            // Draw new selector
            uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, 0, selectedOption + 3);
            Print(L"> ");
        } else if (Key.ScanCode == SCAN_DOWN) {
            // Clear old selector at current position
            uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, 0, selectedOption + 3);
            Print(L"  ");
            
            selectedOption++;
            if (selectedOption > menuItems) {
                selectedOption = 1;
            }
            
            // Draw new selector
            uefi_call_wrapper(SystemTable->ConOut->SetCursorPosition, 3, SystemTable->ConOut, 0, selectedOption + 3);
            Print(L"> ");
        } else if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            // Execute selected option
            uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
            
            switch (selectedOption) {
                case 1:
                    DisplaySystemInfo(SystemTable);
                    break;
                case 2:
                    DisplayGraphicsInfo(SystemTable);
                    break;
                case 3:
                    DisplayBootInfo(SystemTable);
                    break;
                case 4:
                    DisplayMemoryInfo(SystemTable);
                    break;
                case 5:
                    DisplayCurrentTime(SystemTable);
                    break;
                case 6:
                    DisplayFileSystemInfo(SystemTable);
                    break;
                case 7:
                    DisplayUEFIVariables(SystemTable);
                    break;
                case 8:
                    ChangeTextColor(SystemTable);
                    break;
                case 9:
                    DisplayHelp(SystemTable);
                    break;
                case 10:
                    Print(L"\nExiting UEFI application.\n");
                    return EFI_SUCCESS;
                default:
                    break;
            }
            
            Print(L"\nPress any key to return to menu...\n");
            while (uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Key) == EFI_NOT_READY);
            
            uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
            Print(L"\n====== UEFI System Information Tool ======\n");
            Print(L"Use UP/DOWN arrows to select, ENTER to confirm\n\n");
            
            // Print menu items again
            Print(L"> System Info\n");
            Print(L"  Graphics Modes\n");
            Print(L"  Boot Information\n");
            Print(L"  Memory Information\n");
            Print(L"  Current Time\n");
            Print(L"  File Systems\n");
            Print(L"  UEFI Variables\n");
            Print(L"  Text Color\n");
            Print(L"  Help\n");
            Print(L"  Exit\n");
            
            selectedOption = 1;
        }
    }

    return EFI_SUCCESS;
}