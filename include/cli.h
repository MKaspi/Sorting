#ifndef CLI_H
#define CLI_H

#include "dataset.h"

typedef struct {
    char *plugin_path;

    input_mode_t input_mode;
    char *input_path;

    size_t elem_size;
    size_t elem_count;

    int show_help;
    int show_version;
    int show_plugin_detail;
    int print_steps;

} config_t;

config_t parse_cli(int argc, char **argv);
void print_help(void);

#endif

