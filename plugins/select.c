#include "sort.h"
#include "abi.h"
#include <string.h>
#include <stdio.h>

plugin_info_t plugin_info = {
    .name = "select",
    .description = "select sort",
    .version = "1.0",
    .abi_version = FRAMEWORK_ABI_VERSION
};

void sort(sort_ctx *ctx) {
    for (size_t i = 0; i < ctx->n; i++) {
        size_t min = i;
        for (size_t j = i + 1; j < ctx->n; j++) {
            if (ctx->cmp(ctx, j, min) < 0) {
                min = j;
            }
        }
        if (min != i) {
            swap(ctx, i, min);
        }
    }
}