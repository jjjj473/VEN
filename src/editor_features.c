#include "editor_features.h"
#include <ctype.h>
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

int count_words(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return -1;
    }

    int words = 0;
    int in_word = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (isspace(ch)) {
            if (in_word) {
                words++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }
    if (in_word) {
        words++;
    }

    fclose(f);
    return words;
}

long count_bytes(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }

    long bytes = ftell(f);
    fclose(f);
    return bytes;
}
