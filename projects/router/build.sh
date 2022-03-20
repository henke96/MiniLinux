set -e
(cd projects/router/dhcpconf && ./build.sh)
(cd projects/router/init && ./build.sh)
(cd projects/router/iptables && ./build.sh)
echo "Built router!"
