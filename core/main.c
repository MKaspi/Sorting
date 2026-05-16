#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "cli.h"
#include "dataset.h"
#include "sort.h"
#include "abi.h"

int cmp_int(sort_ctx *ctx, size_t left, size_t right) {
    if (ctx->print_steps) {
        printf("compare %ld %ld\n",left,right);
    }

    int *a = (int *)ELEM(ctx, left);
    int *b = (int *)ELEM(ctx, right);

    return (*a > *b) - (*a < *b);
}

int main(int argc, char **argv) {
    config_t cfg = parse_cli(argc, argv);

    if (cfg.show_help || (!cfg.plugin_path && !cfg.dataset_fill)) {
        print_help();
        return 0;
    }

    dataset_t *ds = dataset_create(&cfg);

    if(cfg.dataset_fill){
        dataset_fill(ds);
        dataset_free(ds);
        exit(0);
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

    sort_ctx ctx = {
        .data = ds->data,
        .aux = ds->aux,
        .n = ds->n,
        .elem_size = ds->elem_size,
        .cmp = cmp_int,
        .swap_buf = malloc(ds->elem_size),
        .print_steps = cfg.print_steps
    };

    if (ctx.print_steps) { // vypis prvku
        for(int i=0; i< ctx.n; i++){
            printf("%d ",((int*)ctx.data)[i]);
        }
        printf("\n");
    }

    sort(&ctx);

    free(ctx.swap_buf);
    dataset_free(ds);

    dlclose(handle);
    return 0;
}

