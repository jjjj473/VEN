#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <string.h>

void run(const char *filename) {
    (void)filename; // unused
    struct utsname uts;
    if (uname(&uts) == 0) {
        printf("System: %s %s %s\n", uts.sysname, uts.release, uts.machine);
    }
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    printf("CPU:%s", colon + 1);
                }
                break;
            }
        }
        fclose(f);
    }
    f = fopen("/proc/meminfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "MemTotal", 8) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    printf("Total Memory:%s", colon + 1);
                }
                break;
            }
        }
        fclose(f);
    }
}
