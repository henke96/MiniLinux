#!/bin/sh
set -e

old_brconf=$(cat /proc/sys/net/bridge/bridge-nf-call-iptables)

cleanup() {
    set +e
    echo $old_brconf > /proc/sys/net/bridge/bridge-nf-call-iptables
    ip link del br0
    ip link del tap0
    ip link del tap1
}
if test -n "$1"; then
    trap cleanup EXIT

    ip tuntap add mode tap tap0
    ip link set dev tap0 up
    ip tuntap add mode tap tap1
    ip link set dev tap1 up

    ip link add name br0 type bridge
    ip link set dev br0 up
    ip link set dev tap0 master br0
    ip link set dev "$1" master br0

    echo 0 > /proc/sys/net/bridge/bridge-nf-call-iptables
fi

qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=disk.img -enable-kvm \
-netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device e1000,netdev=mynet0 \
-netdev tap,id=mynet1,ifname=tap1,script=no,downscript=no -device e1000,netdev=mynet1

#qemu-system-x86_64 -kernel ../../misc/linux-5.14.2/arch/x86/boot/bzImage -append "" -drive format=raw,file=disk.img -enable-kvm \
#-netdev tap,id=mynet0,ifname=tap0 -device e1000,netdev=mynet0
