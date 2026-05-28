#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

struct Config {
    int cpu_hz;
    int vf_reset;
    int memory_quirk;
    int clip_quirk;
    int shift_quirk;
    int disp_wait;
    int window_scale;
    uint32_t bg_color;
    uint32_t fg_color;
};

void print_config(struct Config *conf);

#endif
