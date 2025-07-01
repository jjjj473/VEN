#include "editor_features.h"
#include <fstream>
#include <string>

int count_lines(const char *filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return -1;
    }
    int lines = 0;
    std::string line;
    while (std::getline(file, line)) {
        lines++;
    }
    return lines;
}
