#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/x_tables.h>

struct natTarget {
    struct xt_entry_target target;
    struct nf_nat_ipv4_multi_range_compat data;
};

struct natEntry {
    struct ipt_entry entry;
    struct natTarget natTarget;
};

struct standardEntry {
    struct ipt_entry entry;
    struct xt_standard_target standardTarget;
};

struct errorEntry {
    struct ipt_entry entry;
    struct xt_error_target errorTarget;
};

struct replaceEntries {
    struct standardEntry preroutingEntry;
    struct standardEntry inputEntry;
    struct standardEntry outputEntry;
    struct natEntry postroutingNatEntry;
    struct standardEntry postroutingEntry;
    struct errorEntry errorEntry;
};

struct replace {
    struct ipt_replace iptReplace;
    struct replaceEntries replaceEntries;
};

int main(int argc, char **argv) {
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd < 0) {
        printf("socket(AF_INET, SOCK_RAW, IPPROTO_RAW) = %d (%s)\n", fd, strerror(errno));
        return 1;
    }

    struct ipt_getinfo info = {
        .name = "nat"
    };
    socklen_t size = sizeof(info);
    int status = getsockopt(fd, SOL_IP, IPT_SO_GET_INFO, &info, &size);
    if (status < 0) {
        printf("getsockopt(IPT_SO_GET_INFO) = %d (%s)\n", status, strerror(errno));
        return 1;
    }

    char interface[] = "enp6s0";
    struct ipt_ip natIp = {0};
    memcpy(&natIp.outiface, &interface[0], sizeof(interface));
    memset(&natIp.outiface_mask, 0xFF, sizeof(interface));

    struct natEntry natEntry = {
        .entry = {
            .ip = natIp,
            .target_offset	= sizeof(struct ipt_entry),
            .next_offset	= sizeof(struct natEntry),
        },
        .natTarget = {
            .target.u.user = {
                .target_size = sizeof(struct natTarget),
                .name = "MASQUERADE"
            },
            .data = {
                .rangesize = 1
            }
        }
    };

    struct standardEntry standardEntry = {
        .entry = {
            .target_offset	= sizeof(struct ipt_entry),
            .next_offset	= sizeof(struct standardEntry),
        },
        .standardTarget = {
            .target.u.user = {
                .target_size = sizeof(struct xt_standard_target),
                .name = XT_STANDARD_TARGET
            },
            .verdict = -(NF_ACCEPT) - 1
        }
    };

    struct errorEntry errorEntry = {
        .entry = {
            .target_offset	= sizeof(struct ipt_entry),
            .next_offset	= sizeof(struct errorEntry),
        },
        .errorTarget = {
            .target.u.user = {
                .target_size = sizeof(struct xt_error_target),
                .name = XT_ERROR_TARGET
            },
            .errorname = "ERROR"
        }
    };

    // It is okay if malloc fails, kernel just skips copying counter data (but complains in dmesg).
    struct xt_counters *counters = malloc(sizeof(struct xt_counters) * info.num_entries);

    struct replace replace = {
        .iptReplace = {
            .name = "nat",
            .valid_hooks = info.valid_hooks,
            .num_entries = 6,
            .size = sizeof(struct replaceEntries),
            .hook_entry = {
                offsetof(struct replaceEntries, preroutingEntry),
                offsetof(struct replaceEntries, inputEntry),
                0xFFFFFFFF,
                offsetof(struct replaceEntries, outputEntry),
                offsetof(struct replaceEntries, postroutingNatEntry)
            },
            .underflow = {
                offsetof(struct replaceEntries, preroutingEntry),
                offsetof(struct replaceEntries, inputEntry),
                0xFFFFFFFF,
                offsetof(struct replaceEntries, outputEntry),
                offsetof(struct replaceEntries, postroutingEntry)
            },
            .num_counters = info.num_entries,
            .counters = counters
        },
        .replaceEntries = {
            .preroutingEntry = standardEntry,
            .inputEntry = standardEntry,
            .outputEntry = standardEntry,
            .postroutingNatEntry = natEntry,
            .postroutingEntry = standardEntry,
            .errorEntry = errorEntry
        }
    };

    status = setsockopt(fd, SOL_IP, IPT_SO_SET_REPLACE, &replace, sizeof(replace));
    int savedErrno = errno;
    free(counters);
    if (status < 0) {
        printf("setsockopt(IPT_SO_SET_REPLACE) = %d (%s)\n", status, strerror(savedErrno));
        return 1;
    }
    return 0;
}