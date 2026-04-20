#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_run_func)(const char *filename);

int load_and_run_plugin(const char *path, const char *filename);
size_t discover_plugins(const char *plugin_dir, char paths[][512], size_t max_plugins);

#ifdef __cplusplus
}
#endif

#endif // PLUGIN_MANAGER_H
