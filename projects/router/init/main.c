#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>

int main(int argc, char **argv) {
    if (mount("", "/proc", "proc", 0, NULL) < 0) {
        perror("mount(proc)");
        return 1;
    }
    if (mount("", "/sys", "sysfs", 0, NULL) < 0) {
        perror("mount(sysfs)");
        return 1;
    }
    if (mount("", "/dev", "devtmpfs", 0, NULL) < 0) {
        perror("mount(devtmpfs)");
        return 1;
    }
    if (mkdir("/dev/pts", 0777) < 0) {
        perror("mkdir(/dev/pts)");
        return 1;
    }
    if (mount("", "/dev/pts", "devpts", 0, NULL) < 0) {
        perror("mount(devpts)");
        return 1;
    }
    if (mount("", "/tmp", "tmpfs", 0, NULL) < 0) {
        perror("mount(tmpfs)");
        return 1;
    }

    if (setsid() < 0) {
        perror("setsid()");
        return 1;
    }
    if (ioctl(0, TIOCSCTTY, 0) < 0) {
        perror("ioctl(TIOCSCTTY)");
        return 1;
    }

    if (execl("/bin/sh", "sh", "/etc/startup.sh", NULL) < 0) return 9;
    return 0;
}