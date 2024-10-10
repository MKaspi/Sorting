
#include <stdio.h>
#include "Data/Dat.h"


int main(int argc, char* arg[]){
    dat_arr test={.length=100000};

    dat_load(&test);

    dat_print(&test);
    dat_sort(&test);
    printf("\n===\n");
    dat_print(&test);
}
