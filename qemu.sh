#!/bin/sh
# Usage: ./qemu.sh [MACVTAP_INTERFACE]
set -e

cleanup() {
    set +e
    ip link del qemu1
    ip link del qemu2
}

qemu_cmd="qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=disk.img -enable-kvm"
trap cleanup EXIT
trap "" INT # Make sure cleanup gets run on Ctrl-C.

# NIC 1, optionally a macvtap.
if test -n "$1"
then
    ip link add link $1 name qemu1 type macvtap
    qemu_cmd="$qemu_cmd -netdev tap,fd=3,id=net1 -device e1000,netdev=net1,mac=$(cat /sys/class/net/qemu1/address) 3<>/dev/tap$(cat /sys/class/net/qemu1/ifindex)"
else
    ip tuntap add mode tap qemu1
    qemu_cmd="$qemu_cmd -netdev tap,id=net1,ifname=qemu1,script=no,downscript=no -device e1000,netdev=net1"
fi
ip link set qemu1 up

# NIC 2
ip tuntap add mode tap qemu2
ip link set qemu2 up
qemu_cmd="$qemu_cmd -netdev tap,id=net2,ifname=qemu2,script=no,downscript=no -device e1000,netdev=net2"

$qemu_cmd
