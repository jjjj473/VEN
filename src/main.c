#include <stdio.h>
#include <string.h>

#include "editor_features.h"
#include "plugin_manager.h"

#define MAX_PLUGINS 64

static void print_usage(const char *prog) {
    printf("VEN Terminal Developer Toolkit\n");
    printf("Usage:\n");
    printf("  %s <file>                           # quick scan + run all plugins\n", prog);
    printf("  %s scan <file> [plugin|all|none]    # scan file and choose plugin mode\n", prog);
    printf("  %s plugins                          # list available plugins\n", prog);
}

static int run_scan(const char *filename, const char *plugin_mode) {
    const char *plugin_dir = "plugins";
    char plugins[MAX_PLUGINS][512];
    size_t plugin_count = discover_plugins(plugin_dir, plugins, MAX_PLUGINS);

    int lines = count_lines(filename);
    int words = count_words(filename);
    long bytes = count_bytes(filename);

    if (lines < 0 || words < 0 || bytes < 0) {
        fprintf(stderr, "Could not open file '%s'.\n", filename);
        return 1;
    }

    printf("[scan] file=%s\n", filename);
    printf("  lines: %d\n", lines);
    printf("  words: %d\n", words);
    printf("  bytes: %ld\n", bytes);
    printf("[plugins] found=%zu\n", plugin_count);

    if (strcmp(plugin_mode, "none") == 0) {
        return 0;
    }

    if (strcmp(plugin_mode, "all") == 0) {
        for (size_t i = 0; i < plugin_count; i++) {
            printf("\n[plugin] %s\n", plugins[i]);
            load_and_run_plugin(plugins[i], filename);
        }
        return 0;
    }

    for (size_t i = 0; i < plugin_count; i++) {
        if (strstr(plugins[i], plugin_mode) != NULL) {
            printf("\n[plugin] %s\n", plugins[i]);
            return load_and_run_plugin(plugins[i], filename);
        }
    }

    fprintf(stderr, "Plugin mode '%s' not found. Use 'all', 'none', or a plugin name fragment.\n", plugin_mode);
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "plugins") == 0) {
        char plugins[MAX_PLUGINS][512];
        size_t plugin_count = discover_plugins("plugins", plugins, MAX_PLUGINS);
        printf("Available plugins (%zu):\n", plugin_count);
        for (size_t i = 0; i < plugin_count; i++) {
            printf("- %s\n", plugins[i]);
        }
        return 0;
    }

    if (strcmp(argv[1], "scan") == 0) {
        if (argc < 3) {
            print_usage(argv[0]);
            return 1;
        }

        const char *plugin_mode = (argc >= 4) ? argv[3] : "all";
        return run_scan(argv[2], plugin_mode);
    }

    return run_scan(argv[1], "all");
}
