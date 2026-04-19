#include "dataset.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

void dataset_generate(dataset_t *ds) {
    // jen int generator
    for (size_t i = 0; i < ds->n; i++) {
        ((int*)ds->data)[i] = rand();
    }
}

void dataset_mmap(dataset_t *ds, config_t cfg) {
    char *path = cfg.input_path;
    size_t elem_size = cfg.elem_size;

    int fd = open(path, O_RDWR);
    size_t size = lseek(fd, 0, SEEK_END);

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    ds->data = data;
    ds->mapping = data;
    ds->size = size;
    ds->n = size / elem_size;
}


dataset_t dataset_create(config_t cfg){
    dataset_t ds = {0};
    
    ds.elem_size = cfg.elem_size;
    ds.mode = cfg.dataset;
    
    if (ds.mode == DATASET_INTERNAL) {
        // malloc
    } else if(ds.mode == DATASET_FILE) {
        dataset_mmap(&ds,cfg);
    } else {
        printf("fuck off, unknown dataset mode\n");
    }

    return ds;
}


void dataset_free(dataset_t *ds) {
    if (ds->mode == DATASET_INTERNAL) {
        free(ds->data);
    } else if(ds->mode == DATASET_FILE) {
        munmap(ds->mapping, ds->size);
    } else {
        printf("fuck it, uvolni OS\n");
    }
}

