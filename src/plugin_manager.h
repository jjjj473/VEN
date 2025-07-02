#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_run_func)(const char *filename);

int load_and_run_plugin(const char *path, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // PLUGIN_MANAGER_H
