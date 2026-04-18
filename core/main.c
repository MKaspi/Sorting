#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "cli.h"
#include "dataset.h"
#include "sort.h"
#include "abi.h"

int cmp_int(const void *a, const void *b) {
    int x = *(const int*)a;
    int y = *(const int*)b;
    return (x > y) - (x < y);
}

int main(int argc, char **argv) {
    config_t cfg = parse_cli(argc, argv);

    if (cfg.show_help || !cfg.plugin_path) {
        print_help();
        return 0;
    }

    void *handle = dlopen(cfg.plugin_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen failed\n");
        return 1;
    }

    sort_fn sort = (sort_fn)dlsym(handle, "sort");
    plugin_info_t *info = dlsym(handle, "plugin_info");

    if (!sort || !info) {
        fprintf(stderr, "invalid plugin\n");
        return 1;
    }

    if (info->abi_version != FRAMEWORK_ABI_VERSION) {
        fprintf(stderr, "ABI mismatch\n");
        return 1;
    }

    if (cfg.show_plugin_detail) {
        printf("name: %s\n", info->name);
        printf("desc: %s\n", info->description);
        printf("version: %s\n", info->version);
        return 0;
    }

    dataset_t ds;

    if (cfg.input_mode == INPUT_GENERATOR) {
        ds = dataset_generate(cfg.elem_count, cfg.elem_size);
    } else {
        ds = dataset_mmap(cfg.input_path, cfg.elem_size);
    }

    sort_ctx ctx = {
        .data = ds.data,
        .n = ds.n,
        .elem_size = ds.elem_size,
        .cmp = cmp_int,
        .swap_buf = malloc(ds.elem_size)
//        .print_steps = cfg.print_steps
    };

    sort(&ctx);

    free(ctx.swap_buf);
    dataset_free(&ds);

    dlclose(handle);
    return 0;
}

