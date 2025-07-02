#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "editor_features.h"
#include "plugin_manager.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    int lines = count_lines(filename);
    if (lines >= 0) {
        printf("File '%s' has %d lines.\n", filename, lines);
    } else {
        printf("Could not open file '%s'.\n", filename);
        return 1;
    }

    // Demonstrate running a plugin compiled as a shared library.
    // This expects a plugin named plugins/wordcount.so implementing
    // `void run(const char *filename);`
    if (access("plugins/wordcount.so", R_OK) == 0) {
        load_and_run_plugin("plugins/wordcount.so", filename);
    }

    printf("Done.\n");

    return 0;
}
