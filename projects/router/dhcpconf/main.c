#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <sys/syscall.h>

#define PID_FILE "/tmp/udhcpd.pid"

const char dhcpdConf[] =
"start 192.168.0.10\n"
"end 192.168.0.254\n"
"interface lan\n"
"lease_file /tmp/udhcpd.leases\n"
"pidfile " PID_FILE "\n"
"opt router 192.168.0.1\n"
"opt subnet 255.255.255.0\n"
"opt dns %s\n";

const char ifScript[] =
"set -e\n"
"ip route flush 0/0\n"
"ip addr flush dev wan\n"
"ip addr add %s/%s dev wan\n"
"ip route add default via %s\n";

const char ifScriptFlush[] =
"set -e\n"
"ip route flush 0/0\n"
"ip addr flush dev wan\n";

static char buffer[512];

static int run(const char *path, char *const *argv) {
    pid_t pid = fork();
    if (pid == 0) {
        execv(path, argv);
        _exit(137);
    } else if (pid > 0) {
        int wstatus;
        pid_t wpid = waitpid(pid, &wstatus, 0);
        if (wpid < 0) return -1;
        if (!WIFEXITED(wstatus)) return -2;
        return WEXITSTATUS(wstatus);
    }
    return -3;
}

static int updateIfDifferent(const char *path, int32_t contentLen) {
    int fd = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0777);
    if (fd < 0) return -1;

    int status;
    // Check if content is different.
    int32_t pos = 0;
    for (;;) {
        ssize_t num = read(fd, &buffer[contentLen], sizeof(buffer) - contentLen);
        if (num < 0) {
            status = -2;
            goto cleanup_fd;
        }
        int32_t totalRead = pos + num;
        if (num == 0) {
            if (totalRead == contentLen) {
                status = 0;
                goto cleanup_fd;
            }
            break;
        }
        if (totalRead > contentLen || memcmp(&buffer[contentLen], &buffer[pos], num) != 0) break;
        pos = totalRead;
    }
    // It is different, overwrite the file.
    if (lseek(fd, 0, SEEK_SET) < 0) {
        status = -3;
        goto cleanup_fd;
    }

    pos = 0;
    while (pos < contentLen) {
        ssize_t num = write(fd, &buffer[pos], contentLen - pos);
        if (num < 0) {
            status = -4;
            goto cleanup_fd;
        }
        pos += num;
    }

    if (ftruncate(fd, contentLen) < 0) {
        status = -5;
        goto cleanup_fd;
    }

    status = 1;
    cleanup_fd:
    close(fd);
    return status;
}

static int updateIfScript(void) {
    char *ip = getenv("ip");
    char *subnet = getenv("subnet");
    char *gateway = getenv("router");

    int32_t ifScriptLen;
    if (ip != NULL && subnet != NULL && gateway != NULL) {
        ifScriptLen = snprintf(&buffer[0], sizeof(buffer), &ifScript[0], ip, subnet, gateway);
        if (ifScriptLen < 0 || ifScriptLen >= sizeof(buffer)) return -1;
    } else {
        ifScriptLen = sizeof(ifScriptFlush) - 1;
        memcpy(&buffer[0], &ifScriptFlush[0], ifScriptLen);
    }

    int status = updateIfDifferent("/tmp/ifScript.sh", ifScriptLen);
    if (status < 0) {
        printf("updateIfDifferent(/tmp/ifScript.sh) = %d\n", status);
        return -2;
    }
    return status;
}

static int updateDhcpdConf(void) {
    char *dhcpServers = getenv("dns");
    if (dhcpServers == NULL) dhcpServers = "8.8.8.8";

    int32_t confLen = snprintf(&buffer[0], sizeof(buffer), &dhcpdConf[0], dhcpServers);
    if (confLen < 0 || confLen >= sizeof(buffer)) return -1;

    int status = updateIfDifferent("/tmp/dhcpd.conf", confLen);
    if (status < 0) {
        printf("updateIfDifferent(/tmp/dhcpd.conf) = %d\n", status);
        return -2;
    }
    return status;
}

static int readPid(const char *pidFilePath, pid_t *pid) {
    int fd = open(pidFilePath, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) {
            *pid = -1;
            return 0;
        }
        return -1;
    }
    int status;

    int32_t pos = 0;
    for (;;) {
        size_t remainingBuffer = sizeof(buffer) - pos;
        if (remainingBuffer < 2) {
            status = -2;
            goto cleanup_fd;
        }
        ssize_t num = read(fd, &buffer[pos], remainingBuffer - 1);
        if (num < 0) {
            status = -3;
            goto cleanup_fd;
        }
        if (num == 0) break;
        pos += num;
    }
    buffer[pos] = '\0';
    *pid = strtol(&buffer[0], NULL, 10);
    if (*pid <= 0) {
        status = -4;
        goto cleanup_fd;
    }

    status = 0;

    cleanup_fd:
    close(fd);
    return status;
}

static int termAndWait(pid_t pid) {
    int pidFd = syscall(SYS_pidfd_open, pid, 0);
    if (pidFd < 0) {
        return -1;
    }
    int status;
    if (kill(pid, SIGTERM) < 0) {
        status = -2;
        goto cleanup_pidFd;
    }

    struct pollfd pollfd = {
        .fd = pidFd,
        .events = POLLIN
    };
    status = poll(&pollfd, 1, 5000);
    if (status <= 0) {
        status = -3;
        goto cleanup_pidFd;
    }
    cleanup_pidFd:
    close(pidFd);
    return status;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    if (
        strcmp("bound", argv[1]) != 0 &&
        strcmp("renew", argv[1]) != 0 &&
        strcmp("leasefail", argv[1]) != 0
    ) return 0;

    int ifScriptUpdated = updateIfScript();
    if (ifScriptUpdated < 0) return 1;
    printf("ifScriptUpdated = %d\n", ifScriptUpdated);

    int dhcpdConfUpdated = updateDhcpdConf();
    if (dhcpdConfUpdated < 0) return 1;
    printf("dhcpdConfUpdated = %d\n", dhcpdConfUpdated);

    pid_t udhcpdPid;
    int status = readPid(PID_FILE, &udhcpdPid);
    if (status < 0) {
        printf("readPid() = %d\n", status);
        return 1;
    }
    printf("udhcpdPid = %d\n", udhcpdPid);

    if (dhcpdConfUpdated && udhcpdPid != -1) {
        // Kill old udhcpd.
        status = termAndWait(udhcpdPid);
        if (status < 0) {
            printf("termAndWait = %d\n", status);
            return 1;
        }
        printf("Killed udhcpd!\n");
    }

    if (ifScriptUpdated) {
        char *const ifScriptArgv[] = {
            "sh",
            "/tmp/ifScript.sh",
            NULL
        };
        status = run("/bin/sh", &ifScriptArgv[0]);
        if (status != 0) {
            printf("run(ifScript) = %d\n", status);
            return 1;
        }
        printf("Ran ifScript!\n");
    }

    if (dhcpdConfUpdated) {
        char *const udhcpdArgv[] = {
            "udhcpd",
            "-I",
            "192.168.0.1",
            "/tmp/dhcpd.conf",
            NULL
        };
        status = run("/bin/udhcpd", &udhcpdArgv[0]);
        if (status != 0) {
            printf("run(udhcpd) = %d\n", status);
            return 1;
        }
        printf("Started udhcpd!\n");
    }
    return 0;
}