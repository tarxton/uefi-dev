# Build a minimal x64 UEFI application with gnu-efi on WSL
# Workspace: /mnt/c/Users/Administrator/Desktop/uefi-dev

# Paths
SRCDIR   := src
BUILDDIR := build
OUTDIR   := dist
ESPDIR   := esp

# Toolchain
CC      ?= gcc
LD      ?= ld
OBJCOPY ?= objcopy

# gnu-efi locations (your system)
EFIINC  := /usr/include/efi
EFIARCH := $(EFIINC)/x86_64
GNUEFI  := /usr/lib
LIBDIR  := /usr/lib

# Compiler and linker flags
CFLAGS  := -ffreestanding -fshort-wchar -fno-stack-protector \
           -fno-asynchronous-unwind-tables -mno-red-zone \
           -Wall -Wextra -I$(EFIINC) -I$(EFIARCH)
LDFLAGS := -nostdlib -znocombreloc -T $(GNUEFI)/elf_x86_64_efi.lds -shared -Bsymbolic
LIBS    := -L$(LIBDIR) -lefi -lgnuefi

# Target
TARGET    := BOOTX64
EFI_IMAGE := $(OUTDIR)/$(TARGET).EFI

# Source files
SRC := $(SRCDIR)/main.c
OBJ := $(BUILDDIR)/main.o
SO  := $(BUILDDIR)/$(TARGET).so

.PHONY: all clean esp

all: $(EFI_IMAGE)

$(BUILDDIR):
	mkdir -p $@

$(OUTDIR):
	mkdir -p $@

$(OBJ): $(SRC) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SO): $(OBJ)
	$(LD) $(LDFLAGS) $(GNUEFI)/crt0-efi-x86_64.o $< -o $@ $(LIBS)

$(EFI_IMAGE): $(SO) | $(OUTDIR)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
	           -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
	           --target efi-app-x86_64 $< $@
	@echo "Built: $(EFI_IMAGE)"

esp: $(EFI_IMAGE)
	mkdir -p $(ESPDIR)/EFI/BOOT
	cp $(EFI_IMAGE) $(ESPDIR)/EFI/BOOT/BOOTX64.EFI
	@echo "ESP structure created: $(ESPDIR)/EFI/BOOT/BOOTX64.EFI"

clean:
	rm -rf $(BUILDDIR) $(OUTDIR) $(ESPDIR)
	@echo "Clean complete"