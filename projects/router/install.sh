#!/bin/bash

# Config:
# Linux Kernel (built with kernel_config as .config)
KERNEL=../../misc/linux-5.14.2/arch/x86/boot/bzImage
# Busybox (built with defconfig, static build)
BUSYBOX=../../misc/busybox-1.34.0/busybox

DIR="$(dirname "${BASH_SOURCE[0]}")"
MNT_DIR=$DIR/../../mnt

# EFI
mkdir -p $MNT_DIR/boot/EFI/BOOT
cp $KERNEL $MNT_DIR/boot/EFI/BOOT/BOOTX64.EFI

# Userspace
mkdir -p $MNT_DIR/primary/dev
mkdir -p $MNT_DIR/primary/bin
mkdir -p $MNT_DIR/primary/proc
mkdir -p $MNT_DIR/primary/sys

cp $BUSYBOX $MNT_DIR/primary/bin/busybox
cp $DIR/../common/sv-latin1 $MNT_DIR/primary/

cp $DIR/init/init.bin $MNT_DIR/primary/bin/init
