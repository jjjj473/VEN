#include "editor_features.h"
#include <stdio.h>

int count_lines(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return -1;
    }
    int lines = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }
    fclose(f);
    return lines;
}
