#ifndef MARKUP_H
#define MARKUP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MARKUP_COMPONENT_HEADING,
    MARKUP_COMPONENT_PARAGRAPH,
    MARKUP_COMPONENT_LIST_ITEM,
    MARKUP_COMPONENT_BLOCKQUOTE,
    MARKUP_COMPONENT_CODE_BLOCK
} markup_component_type;

typedef struct {
    markup_component_type type;
    int level;
    char *text;
    size_t source_line;
    double smoothness;
    double complexity;
} markup_component;

typedef struct {
    markup_component *items;
    size_t count;
    size_t capacity;
} markup_document;

int markup_parse_file(const char *filename, markup_document *out_doc);
void markup_free_document(markup_document *doc);
char *markup_render_html(const markup_document *doc);
const char *markup_component_type_name(markup_component_type type);

#ifdef __cplusplus
}
#endif

#endif // MARKUP_H
