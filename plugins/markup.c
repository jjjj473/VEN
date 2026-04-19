#include <stdio.h>
#include <stdlib.h>

#include "../src/markup.h"

void run(const char *filename) {
    markup_document doc;
    if (markup_parse_file(filename, &doc) != 0) {
        printf("[markup] Unable to parse %s\n", filename);
        return;
    }

    printf("[markup] Parsed %zu components from %s\n", doc.count, filename);
    for (size_t i = 0; i < doc.count; i++) {
        markup_component *c = &doc.items[i];
        printf("  - line %zu: %-10s smooth=%.2f complexity=%.2f\n",
               c->source_line,
               markup_component_type_name(c->type),
               c->smoothness,
               c->complexity);
    }

    char *html = markup_render_html(&doc);
    if (html) {
        printf("[markup] Render preview:\n%s", html);
        free(html);
    }

    markup_free_document(&doc);
}
