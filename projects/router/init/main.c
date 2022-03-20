#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>

int main(void) {
    if (mount("", "/proc", "proc", 0, NULL) < 0) {
        perror("mount(proc)");
        goto fail;
    }

    // Disable memory overcommit as early as possible.
    int fd = open("/proc/sys/vm/overcommit_memory", O_WRONLY);
    if (fd < 0) {
        perror("open(overcommit_memory)");
        goto fail;
    }
    int num = write(fd, "2", 1);
    close(fd);
    if (num != 1) {
        perror("write(overcommit_memory)");
        goto fail;
    }
    // Panic if we somehow still run out of memory.
    fd = open("/proc/sys/vm/panic_on_oom", O_WRONLY);
    if (fd < 0) {
        perror("open(panic_on_oom)");
        goto fail;
    }
    num = write(fd, "2", 1);
    close(fd);
    if (num != 1) {
        perror("write(panic_on_oom)");
        goto fail;
    }

    if (mount("", "/sys", "sysfs", 0, NULL) < 0) {
        perror("mount(sysfs)");
        goto fail;
    }
    if (mkdir("/dev/pts", 0777) < 0) {
        perror("mkdir(/dev/pts)");
        goto fail;
    }
    if (mount("", "/dev/pts", "devpts", 0, NULL) < 0) {
        perror("mount(devpts)");
        goto fail;
    }
    if (mount("", "/tmp", "tmpfs", 0, NULL) < 0) {
        perror("mount(tmpfs)");
        goto fail;
    }

    // SIGPIPE should not exist.
    if (sigaction(SIGPIPE, &(struct sigaction) { .sa_handler = SIG_IGN }, NULL) < 0) {
        perror("sigaction(SIGPIPE)");
        goto fail;
    }

    if (setsid() < 0) {
        perror("setsid()");
        goto fail;
    }
    if (ioctl(0, TIOCSCTTY, 0) < 0) {
        perror("ioctl(TIOCSCTTY)");
        goto fail;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork()");
        goto fail;
    }
    if (pid == 0) {
        if (execl("/bin/sh", "sh", "/etc/startup.sh", NULL) < 0) return 1;
    } else {
        for (;;) {
            int wstatus;
            pid_t termPid = waitpid(-1, &wstatus, 0);
            if (termPid < 0) {
                perror("waitpid()");
                goto fail;
            }
            printf("PID %d terminated\n", termPid);
        }
    }
    fail:
    sync();
    reboot(RB_AUTOBOOT);
    return 1;
}