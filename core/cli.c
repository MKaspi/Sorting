#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

static dataset_type_t parse_dataset_type(char *value, char **path)
{
    if (strcmp(value, "internal") == 0) {
        *path = NULL;
        return DATASET_INTERNAL;
    }

    *path = value;
    return DATASET_FILE;
}

static size_t parse_size(char *value)
{
    return (size_t)atol(value);
}

config_t parse_cli(int argc, char **argv)
{
    config_t cfg = {0};

    cfg.elem_size = sizeof(int);
    cfg.dataset = DATASET_INTERNAL;
    cfg.aux_dataset = DATASET_NONE;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            cfg.show_help = 1;
        }
        else if (strcmp(argv[i], "--version") == 0) {
            cfg.show_version = 1;
        }
        else if (strcmp(argv[i], "--plugin-detail") == 0) {
            cfg.show_plugin_detail = 1;
        }
        else if (strcmp(argv[i], "--elem-size") == 0) {
            cfg.elem_size = parse_size(argv[++i]);
        }
        else if (strcmp(argv[i], "--elem-count") == 0) {
            cfg.elem_count = parse_size(argv[++i]);
        }
        else if (strcmp(argv[i], "--enlarge-dataset") == 0) {
            cfg.enlarge_dataset = 1;
        }
        else if (strcmp(argv[i], "--print-steps") == 0) {
            cfg.print_steps = 1;
        }
        else if (strcmp(argv[i], "--dataset-fill") == 0) {
            cfg.dataset_fill = 1;
        }
        else if (strcmp(argv[i], "--dataset") == 0) {
            cfg.dataset = parse_dataset_type(argv[++i], &cfg.input_path);
        }
        else if (strcmp(argv[i], "--aux-dataset") == 0) {
            cfg.aux_dataset = parse_dataset_type(argv[++i], &cfg.aux_input_path);
        }
        else {
            cfg.plugin_path = argv[i];
        }
    }

    return cfg;
}

void print_help(void)
{
    printf("Program: sorter\n");
    printf("Version: 2.0.0\n");
    printf("\n");
    printf("Usage:\n");
    printf("  bench <plugin> --elem-count N [options]\n");
    printf("  bench <plugin> --dataset internal --elem-count N [options]\n");
    printf("  bench <plugin> --dataset <file> --elem-count N [options]\n");
    printf("  bench <plugin> --dataset <file> --aux-dataset <file> --elem-count N [options]\n");
    printf("  bench <plugin> --dataset internal --aux-dataset internal --elem-count N [options]\n");
    printf("  bench <plugin> --plugin-detail\n");
    printf("  bench --version\n");
    printf("  bench --help\n");
    printf("\n");
    printf("Required:\n");
    printf("  --elem-count N          Number of elements in main and aux dataset\n");
    printf("\n");
    printf("Dataset options:\n");
    printf("  --dataset internal      Use allocated memory for main dataset\n");
    printf("  --dataset <file>        Use mmap file for main dataset\n");
    printf("  --aux-dataset internal  Use allocated memory for aux dataset\n");
    printf("  --aux-dataset <file>    Use mmap file for aux dataset\n");
    printf("  --enlarge-dataset       Enlarge mmap files when they are smaller than requested\n");
    printf("\n");
    printf("Other options:\n");
    printf("  --elem-size S           Element size in bytes, default is sizeof(int)\n");
    printf("  --dataset-fill          Fill dataset with random values truncated to elem-size\n");
    printf("  --print-steps           Print processing steps\n");
    printf("  --plugin-detail         Print plugin detail\n");
    printf("  --version               Print version\n");
    printf("  --help                  Print this help\n");
}


