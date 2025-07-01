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

    // Launch the Python GUI demonstrating various editor tools
    system("python3 scripts/editor_gui.py");

    return 0;
}
