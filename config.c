#include <stdio.h>
#include "config.h"

void print_config(struct Config *conf) {
    printf("CPU CLOCK: %d Hz\n", conf->cpu_hz * 100);
    
    if (conf->disp_wait) {
        printf("DISP.WAIT: ON\n");
    } else {
        printf("DISP.WAIT: OFF\n");
    }
}
