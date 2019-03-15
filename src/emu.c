#include <stdio.h>
#include "parser.h"
#include "disp.h"

int main (int argc, char *argv[]) {
    if(argc < 2) {
        printf("No rom given!\n");
        return 1;
    }
    rom_load(argv[1]);
    disp_init();
    run();
    return 0;
}