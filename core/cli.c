#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

config_t parse_cli(int argc, char **argv) {
    config_t cfg = {0};

    cfg.elem_size = sizeof(int);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            cfg.show_help = 1;
        }
        else if (strcmp(argv[i], "--version") == 0) {
            cfg.show_help = 1;
        }
        else if (strcmp(argv[i], "--plugin_detail") == 0) {
            cfg.show_plugin_detail = 1;
        }
        else if (strcmp(argv[i], "--elem-size") == 0) {
            cfg.elem_size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--elem-count") == 0) {
            cfg.elem_count = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "--print-steps") == 0) {
            cfg.print_steps = 1;
        }
        else if (strcmp(argv[i], "--input") == 0) {
            char *mode = argv[++i];
            if (strcmp(mode, "generator") == 0) {
                cfg.input_mode = INPUT_GENERATOR;
            } else {
                cfg.input_mode = INPUT_FILE;
                cfg.input_path = mode;
            }
        }
        else {
            cfg.plugin_path = argv[i];
        }
    }

    return cfg;
}

void print_help(void) {
    printf("Program: sorter\n");
    printf("Version: 2.0.0\n");
    printf("Usage:\n");
    printf("bench <plugin> --input generator --elem-count N [--elem-size S]\n");
    printf("bench <plugin> --input <file> [--elem-size S] [--print-steps]\n");
    printf("bench <plugin> --plugin_detail\n");
    printf("bench --version\n");
    printf("bench --help\n");
}

