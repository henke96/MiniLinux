#!/bin/sh
set -e

mkdir -p mnt/primary/bin
mkdir -p mnt/primary/proc
mkdir -p mnt/primary/sys
mkdir -p mnt/primary/etc
mkdir -p mnt/primary/tmp

# Busybox
cp common/third_party/busybox*/busybox mnt/primary/bin/busybox
ln -s /bin/busybox mnt/primary/bin/ls
ln -s /bin/busybox mnt/primary/bin/sh
ln -s /bin/busybox mnt/primary/bin/loadkmap
ln -s /bin/busybox mnt/primary/bin/telnetd
ln -s /bin/busybox mnt/primary/bin/brctl
ln -s /bin/busybox mnt/primary/bin/ip
ln -s /bin/busybox mnt/primary/bin/udhcpc
ln -s /bin/busybox mnt/primary/bin/udhcpd

cp projects/common/sv-latin1 mnt/primary/etc
cp projects/router/etc/startup.sh mnt/primary/etc/
cp projects/router/init/init.bin mnt/primary/bin/init
cp projects/router/iptables/iptables.bin mnt/primary/bin/iptables
cp projects/router/dhcpconf/dhcpconf.bin mnt/primary/bin/dhcpconf
echo "Installed router!"