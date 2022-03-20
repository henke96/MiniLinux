#!/bin/sh
set -e

cleanup() {
    set +e
    umount mnt/boot
    umount mnt/primary
    losetup -d $dev
}

dev="$(losetup --show -P -f disk.img)"
trap cleanup EXIT

mkdir -p mnt/boot
mount ${dev}p1 mnt/boot
mkdir -p mnt/primary
mount ${dev}p2 mnt/primary

echo "Disk device is: $dev"
echo "Entering shell, use Ctrl-D when done."
$SHELL
