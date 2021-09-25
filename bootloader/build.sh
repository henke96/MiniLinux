#!/bin/sh
flags_clang="-target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone \
-I../uefi/Include -I../uefi/Include/X64 \
-nostdlib -Wl,-entry:efiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link"

flags_mingw="-ffreestanding \
-I../uefi/Include -I../uefi/Include/X64 \
-nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efiMain"

x86_64-w64-mingw32-gcc $flags_mingw -o BOOTX64.EFI.gcc main.c
clang $flags_clang -o BOOTX64.EFI.clang main.c