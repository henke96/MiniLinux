#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>

int set_ip(void) {
    struct ifreq ifreq;

    int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) return -1;

    strncpy(ifreq.ifr_name, "eth0", IFNAMSIZ);
    struct sockaddr_in sockaddr_in = {
        .sin_family = AF_INET,
        .sin_port = 0,
        .sin_addr = {
            .s_addr = inet_addr("10.0.0.1")
        }
    };

    char *p = (char *) &sockaddr_in;
    memcpy( (((char *)&ifreq + offsetof(struct ifreq, ifr_addr) )),
                        p, sizeof(struct sockaddr));

    if (ioctl(sockFd, SIOCSIFADDR, &ifreq) < 0) return -2;
    if (ioctl(sockFd, SIOCGIFFLAGS, &ifreq) < 0) return -3;

    ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;

    if (ioctl(sockFd, SIOCSIFFLAGS, &ifreq) < 0) return -4;
    close(sockFd);
}

int dump_ifaces(void) {
    struct ifaddrs *ifaddrs;
    char temp[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddrs) < 0) return -1;

    while (ifaddrs != NULL) {
        if (ifaddrs->ifa_addr != NULL) {
            int family = ifaddrs->ifa_addr->sa_family;
            printf(
                "%-8s %s (%d)\n",
                ifaddrs->ifa_name,
                (family == AF_PACKET) ? "AF_PACKET" :
                (family == AF_INET) ? "AF_INET" :
                (family == AF_INET6) ? "AF_INET6" : "???",
                family
            );
            if (family == AF_INET) {
                struct sockaddr_in *addr_in = (struct sockaddr_in *)ifaddrs->ifa_addr;
                inet_ntop(AF_INET, &(addr_in->sin_addr), &temp[0], INET_ADDRSTRLEN);
                printf("%s\n", &temp[0]);

                addr_in = (struct sockaddr_in *)ifaddrs->ifa_netmask;
                inet_ntop(AF_INET, &(addr_in->sin_addr), &temp[0], INET_ADDRSTRLEN);
                printf("%s\n", &temp[0]);
            } else if (family == AF_INET6) {
                struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)ifaddrs->ifa_addr;
                inet_ntop(AF_INET6, &(addr_in6->sin6_addr), &temp[0], INET6_ADDRSTRLEN);
                printf("%s\n", &temp[0]);
            } else if (family == AF_PACKET) {
                struct sockaddr_ll *addr_ll = (struct sockaddr_ll*)ifaddrs->ifa_addr;
                for (int i = 0; i < addr_ll->sll_halen; ++i) {
                    printf(" %02x", (unsigned char) addr_ll->sll_addr[i]);
                }
                printf("\n");
            }
        }
        ifaddrs = ifaddrs->ifa_next;
    }
    freeifaddrs(ifaddrs);
    return 0;
}

int main(int argc, char **argv) {
    int status = dump_ifaces();
    if (status < 0) printf("status1=%d\n", status);
    status = set_ip();
    if (status < 0) printf("status2=%d\n", status);
    status = dump_ifaces();
    if (status < 0) printf("status3=%d\n", status);
    execl("/bin/chess", "chess", NULL);
    return 0;
}