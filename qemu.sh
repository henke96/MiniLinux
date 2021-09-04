#!/bin/sh
qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=disk.img -enable-kvm