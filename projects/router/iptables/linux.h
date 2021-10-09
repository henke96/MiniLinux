// linux/netfilter.h
enum nf_inet_hooks {
    NF_INET_PRE_ROUTING,
    NF_INET_LOCAL_IN,
    NF_INET_FORWARD,
    NF_INET_LOCAL_OUT,
    NF_INET_POST_ROUTING,
    NF_INET_NUMHOOKS
};

#define NF_ACCEPT 1

// linux/netfilter/xtables.h
#define XT_FUNCTION_MAXNAMELEN 30
#define XT_EXTENSION_MAXNAMELEN 29
#define XT_TABLE_MAXNAMELEN 32
#define XT_STANDARD_TARGET ""
#define XT_ERROR_TARGET "ERROR"

struct xt_entry_target {
    union {
        struct {
            uint16_t target_size;
            char name[XT_EXTENSION_MAXNAMELEN];
            uint8_t revision;
        } user;
        struct {
            uint16_t target_size;
            char *target;
        } kernel;
        uint16_t target_size;
    } u;
};

struct xt_standard_target {
    struct xt_entry_target target;
    int verdict;
};

struct xt_error_target {
    struct xt_entry_target target;
    char errorname[XT_FUNCTION_MAXNAMELEN];
};

struct xt_counters {
    uint64_t pcnt;
    uint64_t bcnt;
};

// linux/netfilter/nf_nat.h
struct nf_nat_ipv4_range {
    unsigned int flags;
    uint32_t min_ip;
    uint32_t max_ip;
    uint16_t min;
    uint16_t max;
};

struct nf_nat_ipv4_multi_range_compat {
    unsigned int rangesize;
    struct nf_nat_ipv4_range range[1];
};

// linux/netfilter_ipv4/ip_tables.h
#define IFNAMSIZ 16
#define IPT_SO_SET_REPLACE 64
#define IPT_SO_GET_INFO 64

struct ipt_ip {
    uint32_t src, dst;
    uint32_t smsk, dmsk;
    char iniface[IFNAMSIZ], outiface[IFNAMSIZ];
    unsigned char iniface_mask[IFNAMSIZ], outiface_mask[IFNAMSIZ];
    uint16_t proto;
    uint8_t flags;
    uint8_t invflags;
};

struct ipt_entry {
    struct ipt_ip ip;
    unsigned int nfcache;
    uint16_t target_offset;
    uint16_t next_offset;
    unsigned int comefrom;
    struct xt_counters counters;
};

struct ipt_replace {
    char name[XT_TABLE_MAXNAMELEN];
    unsigned int valid_hooks;
    unsigned int num_entries;
    unsigned int size;
    unsigned int hook_entry[NF_INET_NUMHOOKS];
    unsigned int underflow[NF_INET_NUMHOOKS];
    unsigned int num_counters;
    struct xt_counters *counters;
};

struct ipt_getinfo {
    char name[XT_TABLE_MAXNAMELEN];
    unsigned int valid_hooks;
    unsigned int hook_entry[NF_INET_NUMHOOKS];
    unsigned int underflow[NF_INET_NUMHOOKS];
    unsigned int num_entries;
    unsigned int size;
};