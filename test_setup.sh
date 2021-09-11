#!/bin/sh

# EFI
mkdir -p mnt/boot/EFI/BOOT
cp ../../misc/linux-5.14.2/arch/x86/boot/bzImage mnt/boot/EFI/BOOT/BOOTX64.EFI

# Userspace
mkdir -p mnt/primary/dev
mkdir -p mnt/primary/bin
cp userspace/init/init.bin mnt/primary/bin/init
cp ../server/chess/server/release_server.bin mnt/primary/bin/chess