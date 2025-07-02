#include <stdio.h>
#include <sys/statvfs.h>

void run(const char *filename) {
    struct statvfs buf;
    const char *path = "/";
    if (filename) path = filename; // check path's filesystem
    if (statvfs(path, &buf) == 0) {
        unsigned long total = buf.f_blocks * buf.f_frsize / 1024;
        unsigned long free = buf.f_bfree * buf.f_frsize / 1024;
        printf("Disk usage for %s: total %lu KB, free %lu KB\n", path, total, free);
    } else {
        perror("statvfs");
    }
}
