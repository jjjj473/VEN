#include "markup.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *dup_trimmed(const char *src) {
    while (*src && isspace((unsigned char)*src)) {
        src++;
    }
    size_t len = strlen(src);
    while (len > 0 && isspace((unsigned char)src[len - 1])) {
        len--;
    }
    char *out = (char *)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

static int ensure_capacity(markup_document *doc) {
    if (doc->count < doc->capacity) {
        return 0;
    }
    size_t next = doc->capacity == 0 ? 8 : doc->capacity * 2;
    markup_component *items =
        (markup_component *)realloc(doc->items, next * sizeof(markup_component));
    if (!items) {
        return -1;
    }
    doc->items = items;
    doc->capacity = next;
    return 0;
}

static int append_component(markup_document *doc,
                            markup_component_type type,
                            int level,
                            const char *content,
                            size_t line_no) {
    if (ensure_capacity(doc) != 0) {
        return -1;
    }

    char *text = dup_trimmed(content);
    if (!text) {
        return -1;
    }

    size_t len = strlen(text);
    size_t punctuation = 0;
    for (size_t i = 0; i < len; i++) {
        if (ispunct((unsigned char)text[i])) {
            punctuation++;
        }
    }

    markup_component *item = &doc->items[doc->count++];
    item->type = type;
    item->level = level;
    item->text = text;
    item->source_line = line_no;
    item->complexity = len > 0 ? ((double)len / 24.0) + ((double)punctuation / 6.0) : 0.0;
    item->smoothness = len > 0 ? 1.0 / (1.0 + item->complexity / 3.0) : 0.0;

    return 0;
}

int markup_parse_file(const char *filename, markup_document *out_doc) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return -1;
    }

    out_doc->items = NULL;
    out_doc->count = 0;
    out_doc->capacity = 0;

    char line[2048];
    size_t line_no = 0;
    int in_code_block = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        line_no++;

        if (strncmp(line, "```", 3) == 0) {
            in_code_block = !in_code_block;
            continue;
        }

        if (in_code_block) {
            if (append_component(out_doc, MARKUP_COMPONENT_CODE_BLOCK, 0, line, line_no) != 0) {
                fclose(f);
                markup_free_document(out_doc);
                return -1;
            }
            continue;
        }

        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (line[0] == '#') {
            int level = 0;
            while (line[level] == '#') {
                level++;
            }
            if (append_component(out_doc,
                                 MARKUP_COMPONENT_HEADING,
                                 level,
                                 line + level,
                                 line_no) != 0) {
                fclose(f);
                markup_free_document(out_doc);
                return -1;
            }
            continue;
        }

        if ((line[0] == '-' || line[0] == '*') && isspace((unsigned char)line[1])) {
            if (append_component(out_doc,
                                 MARKUP_COMPONENT_LIST_ITEM,
                                 0,
                                 line + 2,
                                 line_no) != 0) {
                fclose(f);
                markup_free_document(out_doc);
                return -1;
            }
            continue;
        }

        if (line[0] == '>') {
            if (append_component(out_doc,
                                 MARKUP_COMPONENT_BLOCKQUOTE,
                                 0,
                                 line + 1,
                                 line_no) != 0) {
                fclose(f);
                markup_free_document(out_doc);
                return -1;
            }
            continue;
        }

        if (append_component(out_doc, MARKUP_COMPONENT_PARAGRAPH, 0, line, line_no) != 0) {
            fclose(f);
            markup_free_document(out_doc);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

void markup_free_document(markup_document *doc) {
    if (!doc) {
        return;
    }
    for (size_t i = 0; i < doc->count; i++) {
        free(doc->items[i].text);
    }
    free(doc->items);
    doc->items = NULL;
    doc->count = 0;
    doc->capacity = 0;
}

const char *markup_component_type_name(markup_component_type type) {
    switch (type) {
    case MARKUP_COMPONENT_HEADING:
        return "heading";
    case MARKUP_COMPONENT_PARAGRAPH:
        return "paragraph";
    case MARKUP_COMPONENT_LIST_ITEM:
        return "list_item";
    case MARKUP_COMPONENT_BLOCKQUOTE:
        return "blockquote";
    case MARKUP_COMPONENT_CODE_BLOCK:
        return "code_block";
    default:
        return "unknown";
    }
}

char *markup_render_html(const markup_document *doc) {
    size_t capacity = (doc->count + 1) * 256;
    char *rendered = (char *)malloc(capacity);
    if (!rendered) {
        return NULL;
    }
    rendered[0] = '\0';

    for (size_t i = 0; i < doc->count; i++) {
        const markup_component *c = &doc->items[i];
        char row[512];
        switch (c->type) {
        case MARKUP_COMPONENT_HEADING:
            snprintf(row,
                     sizeof(row),
                     "<h%d data-smooth=\"%.2f\">%s</h%d>\n",
                     c->level,
                     c->smoothness,
                     c->text,
                     c->level);
            break;
        case MARKUP_COMPONENT_LIST_ITEM:
            snprintf(row, sizeof(row), "<li data-complexity=\"%.2f\">%s</li>\n", c->complexity, c->text);
            break;
        case MARKUP_COMPONENT_BLOCKQUOTE:
            snprintf(row, sizeof(row), "<blockquote>%s</blockquote>\n", c->text);
            break;
        case MARKUP_COMPONENT_CODE_BLOCK:
            snprintf(row, sizeof(row), "<pre><code>%s</code></pre>\n", c->text);
            break;
        case MARKUP_COMPONENT_PARAGRAPH:
        default:
            snprintf(row, sizeof(row), "<p>%s</p>\n", c->text);
            break;
        }

        size_t needed = strlen(rendered) + strlen(row) + 1;
        if (needed > capacity) {
            size_t next = capacity * 2;
            while (next < needed) {
                next *= 2;
            }
            char *grown = (char *)realloc(rendered, next);
            if (!grown) {
                free(rendered);
                return NULL;
            }
            rendered = grown;
            capacity = next;
        }

        strcat(rendered, row);
    }

    return rendered;
}
