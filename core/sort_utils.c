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

void copy(sort_ctx *ctx, size_t src, size_t dst, copy_direction_t dir){
    if(dir == aux_to_main){
        if (ctx->print_steps) {
            printf("copy aux_to_main %ld %ld\n", src, dst);
        }

        memcpy(ELEM(ctx, dst), ELEM_AUX(ctx, src), ctx->elem_size);
    } else {
        if (ctx->print_steps) {
            printf("copy main_to_aux %ld %ld\n", src, dst);
        }

        memcpy(ELEM_AUX(ctx, dst), ELEM(ctx, src), ctx->elem_size);
    }
}
