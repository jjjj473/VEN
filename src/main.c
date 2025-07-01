#include <stdio.h>
#include <stdlib.h>
#include "editor_features.h"

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

    // Launch the Python GUI demonstrating various editor tools if a display is
    // available. This prevents failures when $DISPLAY is not set, such as in
    // headless environments.
    const char *display = getenv("DISPLAY");
    if (display && *display) {
        system("python3 scripts/editor_gui.py");
    } else {
        printf("No DISPLAY found; skipping GUI launch.\n");
    }

    return 0;
}
