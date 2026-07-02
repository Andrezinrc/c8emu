#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

struct Config {
    int cpu_hz;
    int cpu_trace;
    int vf_reset;
    int memory_quirk;
    int clip_quirk;
    int shift_quirk;
    int disp_wait;
    int window_scale;
};

void print_config(struct Config *conf);

#endif
