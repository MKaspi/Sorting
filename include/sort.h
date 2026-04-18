#ifndef SORT_H
#define SORT_H

#include <stddef.h>
#define ELEM(ctx, i) ((char*)(ctx->data) + (i) * (ctx->elem_size))

typedef int (*cmp_fn)(const void *, const void *);

typedef struct {
    void *data;
    size_t n;
    size_t elem_size;
    cmp_fn cmp;
    void *swap_buf;
    int print_steps;
} sort_ctx;

typedef void (*sort_fn)(sort_ctx *);
void swap(sort_ctx *ctx, size_t left, size_t right);

#endif
