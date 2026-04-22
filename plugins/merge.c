#include "sort.h"
#include "abi.h"
#include <string.h>
#include <stdio.h>

plugin_info_t plugin_info = {
    .name = "merge",
    .description = "merge sort",
    .version = "1.0",
    .abi_version = FRAMEWORK_ABI_VERSION
};

static void merge(sort_ctx *ctx, size_t left, size_t mid, size_t right) {
    size_t i = left;
    size_t j = mid;
    size_t k = left;

    // merge do pomocného pole (aux)
    while (i < mid && j < right) {
        if (ctx->cmp(ctx, i, j) <= 0) {
            copy(ctx, i, k, main_to_aux);
            i++;
        } else {
            copy(ctx, j, k, main_to_aux);
            j++;
        }
        k++;
    }

    // zbytek levé poloviny
    while (i < mid) {
        copy(ctx, i, k, main_to_aux);
        i++;
        k++;
    }

    // zbytek pravé poloviny
    while (j < right) {
        copy(ctx, j, k, main_to_aux);
        j++;
        k++;
    }

    // zkopírovat zpět do původního pole
    for (size_t idx = left; idx < right; idx++) {
        copy(ctx, idx, idx, aux_to_main);
    }
}

static void merge_sort_rec(sort_ctx *ctx, size_t left, size_t right) {
    if (right - left <= 1) return;

    size_t mid = left + (right - left) / 2;

    merge_sort_rec(ctx, left, mid);
    merge_sort_rec(ctx, mid, right);

    merge(ctx, left, mid, right);
}

void sort(sort_ctx *ctx) {
    if (ctx->n <= 1) return;
    merge_sort_rec(ctx, 0, ctx->n);
}

