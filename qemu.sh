#!/bin/sh
qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=disk.img -enable-kvm \
-netdev tap,id=mynet0,ifname=tap0 -device e1000,netdev=mynet0

#qemu-system-x86_64 -kernel ../../misc/linux-5.14.2/arch/x86/boot/bzImage -append "" -drive format=raw,file=disk.img -enable-kvm \
#-netdev tap,id=mynet0,ifname=tap0 -device e1000,netdev=mynet0
