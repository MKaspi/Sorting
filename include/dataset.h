#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

typedef enum {
    INPUT_GENERATOR,
    INPUT_FILE
} input_mode_t;

typedef struct {
    void *data;
    size_t n;
    size_t elem_size;
    input_mode_t mode;

    // pro mmap
    void *mapping;
    size_t size;
} dataset_t;

dataset_t dataset_generate(size_t n, size_t elem_size);
dataset_t dataset_mmap(const char *path, size_t elem_size);
void dataset_free(dataset_t *ds);

#endif

