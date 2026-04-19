#include "sort.h"
#include "abi.h"
#include <string.h>
#include <stdio.h>

plugin_info_t plugin_info = {
    .name = "quick",
    .description = "quick sort",
    .version = "1.0",
    .abi_version = FRAMEWORK_ABI_VERSION
};

static void quicksort(sort_ctx *ctx, size_t left, size_t right) {
    if (left >= right) return;

    size_t pivot = right;
    size_t i = left;

    for (size_t j = left; j < right; j++) {
        if (ctx->cmp(ctx, j, pivot) < 0) {
            swap(ctx, i, j);
            i++;
        }
    }
    swap(ctx, i, pivot);

    if (i > 0) quicksort(ctx, left, i - 1);
    quicksort(ctx, i + 1, right);
}

void sort(sort_ctx *ctx) {
    if (ctx->n > 0) {
        quicksort(ctx, 0, ctx->n - 1);
    }
}