#ifndef CONFIG_H
#define CONFIG_H

struct Config {
    int cpu_hz;
    int disp_wait;
};

void print_config(struct Config *conf);

#endif
