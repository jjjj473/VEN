#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef struct Var {
    char *name;
    char *value;
    struct Var *next;
} Var;

static Var *set_var(Var **ctx, const char *name, const char *value) {
    Var *v = *ctx;
    while (v) {
        if (strcmp(v->name, name) == 0) {
            free(v->value);
            v->value = strdup(value);
            return v;
        }
        v = v->next;
    }
    v = malloc(sizeof(Var));
    v->name = strdup(name);
    v->value = strdup(value);
    v->next = *ctx;
    *ctx = v;
    return v;
}

static const char *get_var(Var *ctx, const char *name) {
    for (Var *v = ctx; v; v = v->next) {
        if (strcmp(v->name, name) == 0)
            return v->value;
    }
    return "";
}

static char *substitute(const char *text, Var *ctx) {
    size_t len = strlen(text);
    char *out = malloc(len * 2 + 1); // rough estimate
    size_t o = 0;
    for (size_t i = 0; i < len; ) {
        if (text[i] == '$' && text[i+1] == '{') {
            i += 2;
            size_t start = i;
            while (text[i] && text[i] != '}') i++;
            char name[64];
            size_t n = i - start;
            if (n >= sizeof(name)) n = sizeof(name)-1;
            strncpy(name, text + start, n);
            name[n] = '\0';
            const char *val = get_var(ctx, name);
            size_t vlen = strlen(val);
            memcpy(out + o, val, vlen);
            o += vlen;
            if (text[i] == '}') i++;
        } else {
            out[o++] = text[i++];
        }
    }
    out[o] = '\0';
    return out;
}

static void unescape(char *s) {
    char *p = s, *o = s;
    while (*p) {
        if (*p == '\\') {
            p++;
            if (*p == 'n') { *o++ = '\n'; p++; }
            else if (*p == 't') { *o++ = '\t'; p++; }
            else { *o++ = *p++; }
        } else {
            *o++ = *p++;
        }
    }
    *o = '\0';
}

static void trim(char *s) {
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    memmove(s, p, strlen(p) + 1);
    for (int i = strlen(s)-1; i >= 0 && isspace((unsigned char)s[i]); i--) s[i] = '\0';
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

static int interpret_lines(char **lines, int count, int *index, Var **ctx, const char *dir, FILE *out);

static void render(const char *path, Var **ctx, FILE *out) {
    char *text = read_file(path);
    if (!text) {
        fprintf(stderr, "Could not open %s\n", path);
        return;
    }
    const char *dir = strrchr(path, '/');
    char dirbuf[256];
    if (dir) {
        size_t dlen = dir - path;
        strncpy(dirbuf, path, dlen);
        dirbuf[dlen] = '\0';
    } else {
        strcpy(dirbuf, ".");
    }
    const char *p = text;
    while (*p) {
        const char *tag = strstr(p, "<?VEN");
        if (!tag) {
            fputs(p, out);
            break;
        }
        fwrite(p, 1, tag - p, out);
        const char *end = strstr(tag, "?>");
        if (!end) break;
        int expr = (tag[5] == '=');
        const char *code_start = tag + (expr ? 6 : 5);
        char *code = strndup(code_start, end - code_start);
        trim(code);
        if (expr) {
            const char *val = get_var(*ctx, code);
            fputs(val, out);
        } else {
            // split code by newlines
            int line_count = 0;
            for (char *c = code; *c; c++) if (*c == '\n') line_count++;
            line_count++; // for last line
            char **lines = malloc(sizeof(char*) * line_count);
            int idx = 0;
            char *saveptr = NULL;
            char *token = strtok_r(code, "\n", &saveptr);
            while (token) {
                trim(token);
                lines[idx++] = token;
                token = strtok_r(NULL, "\n", &saveptr);
            }
            int i = 0;
            interpret_lines(lines, idx, &i, ctx, dirbuf, out);
            free(lines);
        }
        free(code);
        p = end + 2;
    }
    free(text);
}

static void execute_command(const char *cmd, Var **ctx, const char *dir, FILE *out) {
    if (strncmp(cmd, "set ", 4) == 0) {
        const char *rest = cmd + 4;
        char name[64];
        const char *space = strchr(rest, ' ');
        if (!space) return;
        size_t n = space - rest;
        if (n >= sizeof(name)) n = sizeof(name)-1;
        strncpy(name, rest, n);
        name[n] = '\0';
        const char *val = space + 1;
        size_t vlen = strlen(val);
        if ((val[0] == '"' && vlen > 1 && val[vlen-1] == '"') ||
            (val[0] == '\'' && vlen > 1 && val[vlen-1] == '\'')) {
            char *tmp = strndup(val + 1, vlen - 2);
            set_var(ctx, name, tmp);
            free(tmp);
        } else {
            set_var(ctx, name, val);
        }
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        const char *txt = cmd + 5;
        size_t len = strlen(txt);
        if ((txt[0] == '"' && len > 1 && txt[len-1] == '"') ||
            (txt[0] == '\'' && len > 1 && txt[len-1] == '\'')) {
            txt = strndup(txt + 1, len - 2);
        } else {
            txt = strdup(txt);
        }
        char *out_text = substitute(txt, *ctx);
        free((char*)txt);
        unescape(out_text);
        fputs(out_text, out);
        free(out_text);
    } else if (strncmp(cmd, "include ", 8) == 0) {
        const char *file = cmd + 8;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, file);
        render(path, ctx, out);
    }
}

static int interpret_lines(char **lines, int count, int *index, Var **ctx, const char *dir, FILE *out) {
    while (*index < count) {
        char *line = lines[*index];
        if (strncmp(line, "for ", 4) == 0) {
            int start, end;
            char var[32];
            if (sscanf(line+4, "%31s %d %d", var, &start, &end) != 3) {
                (*index)++; continue;
            }
            (*index)++; // body start
            int body_start = *index;
            int depth = 1;
            while (*index < count && depth > 0) {
                if (strncmp(lines[*index], "for ", 4) == 0) depth++;
                else if (strcmp(lines[*index], "endfor") == 0) depth--;
                (*index)++;
            }
            int body_end = *index - 1; // points to endfor
            for (int i = start; i <= end; i++) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%d", i);
                set_var(ctx, var, buf);
                int j = 0;
                interpret_lines(lines + body_start, body_end - body_start, &j, ctx, dir, out);
            }
        } else if (strcmp(line, "endfor") == 0) {
            (*index)++; return 0; // end loop
        } else {
            execute_command(line, ctx, dir, out);
            (*index)++;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.ven>\n", argv[0]);
        return 1;
    }
    Var *ctx = NULL;
    char timebuf[64];
    time_t t = time(NULL);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    set_var(&ctx, "time", timebuf);
    render(argv[1], &ctx, stdout);
    return 0;
}
