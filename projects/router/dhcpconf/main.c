#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PID_FILE "/tmp/udhcpd.pid"

const char configTemplate[] =
"start 192.168.0.10\n"
"end 192.168.0.254\n"
"interface lan\n"
"lease_file /tmp/udhcpd.leases\n"
"pidfile " PID_FILE "\n"
"opt router 192.168.0.1\n"
"opt subnet 255.255.255.0\n"
"opt dns %s\n";

static int run(const char *path, char *const *argv) {
    pid_t pid = fork();
    if (pid == 0) {
        execv(path, argv);
    } else if (pid > 0) {
        int wstatus;
        pid_t wpid = waitpid(pid, &wstatus, 0);
        if (wpid < 0) return -1;
        if (!WIFEXITED(wstatus)) return -2;
        return WEXITSTATUS(wstatus);
    }
    return -3;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    char buffer[512];

    if (strcmp("bound", argv[1]) == 0 || strcmp("renew", argv[1]) == 0) {
        char *ip = getenv("ip");
        if (ip == NULL) return 1;
        char *netmask = getenv("subnet");
        if (netmask == NULL) return 1;
        char *router = getenv("router");
        if (router == NULL) return 1;

        int numWritten = snprintf(&buffer[0], sizeof(buffer), "%s/%s", ip, netmask);
        if (numWritten < 0 || numWritten >= sizeof(buffer)) return 1;

        char *const addrArgv[] = {
            "ip",
            "addr",
            "replace",
            &buffer[0],
            "dev",
            "wan",
            NULL
        };
        int status = run("/bin/ip", &addrArgv[0]);
        if (status != 0) return 1;

        int32_t i = 0;
        for (char *it = &router[0]; *it != '\0' && *it != ' '; ++it) {
            if (i + 1 >= sizeof(buffer)) return 1;
            buffer[i++] = *it;
        }
        buffer[i] = '\0';

        char *const routeArgv[] = {
            "ip",
            "route",
            "replace",
            "default",
            "via",
            &buffer[0],
            "dev",
            "wan",
            NULL
        };
        status = run("/bin/ip", &routeArgv[0]);
        if (status != 0) return 1;
    } else if (strcmp("leasefail", argv[1]) == 0) {
    } else return 0;

    char *dhcpServers = getenv("dns");
    if (dhcpServers == NULL) dhcpServers = "8.8.8.8";

    int numWritten = snprintf(&buffer[0], sizeof(buffer), &configTemplate[0], dhcpServers);
    if (numWritten < 0 || numWritten >= sizeof(buffer)) return 1;

    int fd = open("/tmp/dhcpd.conf", O_CREAT | O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        perror("open()");
        return 1;
    }
    size_t written = 0;
    while (written < numWritten) {
        ssize_t num = write(fd, &buffer[written], numWritten - written);
        if (num < 0) {
            perror("write()");
            return 1;
        }
        written += num;
    }
    close(fd);

    char *const udhcpdArgv[] = {
        "udhcpd",
        "-I",
        "192.168.0.1",
        "/tmp/dhcpd.conf",
        NULL
    };
    // TODO: will start multiple of this, etc.
    int status = run("/bin/udhcpd", &udhcpdArgv[0]);
    printf("GOGOGO %d\n", status);
    return 0;
}