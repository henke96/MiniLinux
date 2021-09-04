#!/bin/sh
set -e

cleanup() {
    set +e
    umount mnt/boot
    umount mnt/primary
    rm -rf mnt
    losetup -d $dev
}

dd if=/dev/zero of=disk.img bs=512 count=204800

dev="$(losetup --show -f disk.img)"
trap cleanup EXIT

parted -s $dev \
mklabel gpt \
mkpart EFI 1Mib 34Mib set 1 esp on \
mkpart Primary 34Mib 99Mib

mkfs -t fat -F 32 ${dev}p1
mkfs -t ext4 ${dev}p2

mkdir -p mnt/boot
mount ${dev}p1 mnt/boot
mkdir -p mnt/primary
mount ${dev}p2 mnt/primary

echo "Disk device is: $dev"
echo "Entering shell, use Ctrl-D when done..."
bash
