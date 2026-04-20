#include "plugin_manager.h"

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

int load_and_run_plugin(const char *path, const char *filename) {
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load plugin %s: %s\n", path, dlerror());
        return 1;
    }

    dlerror();
    plugin_run_func run = (plugin_run_func)dlsym(handle, "run");
    const char *err = dlerror();
    if (err != NULL || !run) {
        fprintf(stderr, "Plugin %s missing run(): %s\n", path, err);
        dlclose(handle);
        return 1;
    }

    run(filename);
    dlclose(handle);
    return 0;
}

size_t discover_plugins(const char *plugin_dir, char paths[][512], size_t max_plugins) {
    DIR *dir = opendir(plugin_dir);
    if (!dir) {
        return 0;
    }

    size_t count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_plugins) {
        size_t name_len = strlen(entry->d_name);
        if (name_len < 4 || strcmp(entry->d_name + name_len - 3, ".so") != 0) {
            continue;
        }

        snprintf(paths[count], 512, "%s/%s", plugin_dir, entry->d_name);
        count++;
    }

    closedir(dir);
    return count;
}
