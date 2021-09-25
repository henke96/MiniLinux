#include <stddef.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/x_tables.h>
#include <stdio.h>

struct nat_target {
    struct xt_entry_target target;
    struct nf_nat_ipv4_multi_range_compat data;
};

struct nat_entry {
    struct ipt_entry entry;
    struct nat_target nat_target;
};

struct standard_entry {
    struct ipt_entry entry;
    struct xt_standard_target standard_target;
};

struct error_entry {
    struct ipt_entry entry;
    struct xt_error_target error_target;
};

struct replace_entries {
    struct standard_entry prerouting_entry;
    struct standard_entry input_entry;
    struct standard_entry output_entry;
    struct standard_entry postrouting_entry;
    struct error_entry error_entry;
};

struct replace {
    struct ipt_replace ipt_replace;
    struct replace_entries replace_entries;
};



int main(int argc, char **argv) {
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd < 0) return 1;

    struct ipt_getinfo info = {
        .name = "nat"
    };
    socklen_t size = sizeof(info);
    int status = getsockopt(fd, SOL_IP, IPT_SO_GET_INFO, &info, &size);
    if (status < 0) {
        printf("getsockopt(GET_INFO) = %d\n", status);
        return 1;
    }

    struct nat_entry nat_entry = {
        .entry = {
            .target_offset	= sizeof(struct ipt_entry),
            .next_offset	= sizeof(struct nat_entry),
        },
        .nat_target = {
            .target.u.user = {
                .target_size = sizeof(struct nat_target),
                .name = "MASQUERADE"
            },
            .data = {
                .rangesize = 1
            }
        }
    };

    struct standard_entry standard_entry = {
        .entry = {
            .target_offset	= sizeof(struct ipt_entry),
            .next_offset	= sizeof(struct standard_entry),
        },
        .standard_target = {
            .target.u.user = {
                .target_size = sizeof(struct xt_standard_target),
                .name = XT_STANDARD_TARGET
            },
            .verdict = -(NF_ACCEPT) - 1
        }
    };

    struct error_entry error_entry = {
        .entry = {
            .target_offset	= sizeof(struct ipt_entry),
            .next_offset	= sizeof(struct error_entry),
        },
        .error_target = {
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
        .ipt_replace = {
            .name = "nat",
            .valid_hooks = info.valid_hooks,
            .num_entries = 5,
            .size = sizeof(struct replace_entries),
            .hook_entry = {
                offsetof(struct replace_entries, prerouting_entry),
                offsetof(struct replace_entries, input_entry),
                0xFFFFFFFF,
                offsetof(struct replace_entries, output_entry),
                offsetof(struct replace_entries, postrouting_entry)
            },
            .underflow = {
                offsetof(struct replace_entries, prerouting_entry),
                offsetof(struct replace_entries, input_entry),
                0xFFFFFFFF,
                offsetof(struct replace_entries, output_entry),
                offsetof(struct replace_entries, postrouting_entry)
            },
            .num_counters = info.num_entries,
            .counters = counters
        },
        .replace_entries = {
            .prerouting_entry = standard_entry,
            .input_entry = standard_entry,
            .output_entry = standard_entry,
            .postrouting_entry = standard_entry,
            .error_entry = error_entry
        }
    };

    status = setsockopt(fd, SOL_IP, IPT_SO_SET_REPLACE, &replace, sizeof(replace));
    free(counters);
    if (status < 0) {
        printf("setsockopt(SET_REPLACE) = %d\n", status);
        return 1;
    }
    return 0;
}