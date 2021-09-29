set -e
loadkmap < /etc/sv-latin1

# LAN
brctl addbr lan
brctl addif lan eth1
ip addr add 192.168.0.1/24 dev lan
ip link set dev lan up
ip link set dev eth1 up

# WAN
brctl addbr wan
brctl addif wan eth0
iptables wan
echo "1" > /proc/sys/net/ipv4/ip_forward
ip link set dev wan up
ip link set dev eth0 up

udhcpc -b -R -i wan -s /bin/dhcpconf

telnetd -b 192.168.0.1 -l /bin/sh
exec sh