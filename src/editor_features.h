#ifndef FEATURES_H
#define FEATURES_H

#ifdef __cplusplus
extern "C" {
#endif

int count_lines(const char *filename);
int count_words(const char *filename);
long count_bytes(const char *filename);

#ifdef __cplusplus
}
#endif

#endif // FEATURES_H
