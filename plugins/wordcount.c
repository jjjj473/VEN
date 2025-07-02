#include <stdio.h>
#include <ctype.h>

void run(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Plugin wordcount: could not open %s\n", filename);
        return;
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
    if (in_word) words++;
    fclose(f);
    printf("Word count: %d\n", words);
}
