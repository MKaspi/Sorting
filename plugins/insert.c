#include "sort.h"
#include "abi.h"
#include <string.h>
#include <stdio.h>

plugin_info_t plugin_info = {
    .name = "insert",
    .description = "insert sort",
    .version = "1.0",
    .abi_version = FRAMEWORK_ABI_VERSION
};

void sort(sort_ctx *ctx) {
    for (size_t i = 1; i < ctx->n; i++) {
        size_t j = i;
        while (j > 0 && ctx->cmp(ctx, j - 1, j) > 0) {
            swap(ctx, j - 1, j);
            j--;
        }
    }
}
