#include <stdio.h>
#include <stdlib.h>

void run(const char *filename) {
    char command[4096];
    int written = snprintf(command, sizeof(command),
                           "node scripts/markup.js \"%s\"", filename);
    if (written < 0 || (size_t)written >= sizeof(command)) {
        printf("[markup] Command construction failed for file: %s\n", filename);
        return;
    }

    int status = system(command);
    if (status != 0) {
        printf("[markup] JS markup engine failed (status=%d). Ensure Node.js is installed.\n",
               status);
    }
}
