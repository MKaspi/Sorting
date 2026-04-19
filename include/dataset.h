#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

#include "dataset_enum.h"
#include "cli.h"

typedef struct {
    void *data;
    size_t n;
    size_t elem_size;
    dataset_type_t mode;

    // pro mmap
    void *mapping; // to si sem dal terminator
    size_t size;
} dataset_t;

dataset_t dataset_create(config_t cfg);
void dataset_generate(dataset_t *ds);
void dataset_free(dataset_t *ds);

#endif

