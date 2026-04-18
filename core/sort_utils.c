#include "sort.h"
#include <string.h>
#include <stdio.h>

void swap(sort_ctx *ctx, size_t left, size_t right) {
    if (ctx->print_steps) {
        printf("swap %zu %zu\n", left, right);
    }

    if (left == right) return;

    char *a = ELEM(ctx, left);
    char *b = ELEM(ctx, right);

    memcpy(ctx->swap_buf, a, ctx->elem_size);
    memcpy(a, b, ctx->elem_size);
    memcpy(b, ctx->swap_buf, ctx->elem_size);
}

