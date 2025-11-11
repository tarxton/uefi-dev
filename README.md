# Simple UEFI Application

This project is a minimal **UEFI application** written in C using the **GNU-EFI** framework.  
It was developed as part of our **Operating Systems** university course to explore low-level boot environments and the UEFI specification.

---

## Overview

The project demonstrates how to:
- Set up a UEFI development environment on Windows using WSL.
- Build a simple EFI executable (`.efi`) with `gnu-efi`.
- Boot and test it inside a virtual machine using `QEMU` and `OVMF` firmware.

Our current implementation displays a simple message on screen when executed by the UEFI firmware, while providing additional functions like changing the text and background color, checking the current time, etc.

---

## Project Structure

uefi-dev/
├── src/
│ └── main.c # UEFI app source code
├── build/ # Build output (object, ELF, and EFI files)
├── esp/
│ └── EFI/
│ └── BOOT/
│ └── BOOTX64.EFI # Final bootable EFI application
├── ovmf/ # OVMF flash device files
├── Makefile
└── README.md

---

## Notes

- OVMF.fd is the open-source UEFI firmware used by QEMU.
- The gnu-efi package provides the necessary headers and startup code.
- The default boot path for x86_64 EFI executables is EFI/BOOT/BOOTX64.EFI.
- The build system is designed for WSL (Ubuntu) but should also work on native Linux.

---

Made for CS307 - Operating Systems (Fall 2025)

## International University of Sarajevo
