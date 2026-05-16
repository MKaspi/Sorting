#ifndef CLI_H
#define CLI_H

#include <stddef.h>

#include "dataset_enum.h"

typedef struct {
    char *plugin_path; // testovany sort

    // Nastaveni datasetu
    dataset_type_t dataset; // typ main datasetu
    char *input_path; // pripadna cesta

    dataset_type_t aux_dataset; // typ aux datasetu
    char *aux_input_path; // priadna cesta

    size_t elem_size; // velikost jednoho prvku
    size_t elem_count; // pocet prvku

    int enlarge_dataset; // povoleni zvetseni souboru

    // akce programu
    int show_help;
    int show_version;
    int show_plugin_detail;
    int print_steps;
    int dataset_fill;
} config_t;

config_t parse_cli(int argc, char **argv);
void print_help(void);

#endif

