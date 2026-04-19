#include "dataset.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

void dataset_fill(dataset_t *ds) {
    // jen int generator
    for (size_t i = 0; i < ds->n; i++) {
        ((int*)ds->data)[i] = rand();
    }
}

void dataset_mmap(dataset_t *ds, config_t cfg) {
    char *path = cfg.input_path;

    int fd = open(path, O_RDWR);
    size_t size = lseek(fd, 0, SEEK_END);

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    ds->data = data;
    ds->mapping = data;
    ds->size = size;
}

void dataset_malloc(dataset_t *ds, config_t cfg) {
    ds->size = ds->elem_size * cfg.elem_count;
    
    void *data = malloc(ds->size);
    if(data == NULL){printf("fuckoff, malloc fail\n");exit(4);}

    ds->data = data;
}


dataset_t dataset_create(config_t cfg){
    dataset_t ds = {0};
    
    ds.elem_size = cfg.elem_size;
    ds.mode = cfg.dataset;
    // velikost alokovanyho prostoru si nastavim pro kazdej zvlast
    // pocet prvku si nastavim potom, protoze se nacte z velikosti souboru

    if (ds.mode == DATASET_INTERNAL) {
        dataset_malloc(&ds,cfg);
    } else if(ds.mode == DATASET_FILE) {
        dataset_mmap(&ds,cfg);
    } else {
        printf("fuck off, unknown dataset mode\n");
    }
    
    ds.n = ds.size / ds.elem_size;

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

