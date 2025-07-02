#include "plugin_manager.h"
#include <dlfcn.h>
#include <stdio.h>

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
