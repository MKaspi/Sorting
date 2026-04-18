#include "sort.h"
#include "abi.h"
#include <string.h>

plugin_info_t plugin_info = {
    .name = "bubble",
    .description = "bubble sort",
    .version = "1.0",
    .abi_version = FRAMEWORK_ABI_VERSION
};

void sort(sort_ctx *ctx) {
    for (size_t i = 0; i < ctx->n; i++) {
        for (size_t j = 0; j + 1 < ctx->n - i; j++) {
            void *a = ELEM(ctx, j);
            void *b = ELEM(ctx, j+1);

            if (ctx->cmp(a, b) > 0) {
                swap(ctx, j, j+1);
            }
        }
    }
}

