#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef enum {
    MODE_CHIP8,
    MODE_SCHIP,
    MODE_XOCHIP
} EmulatorMode;

struct Config {
    EmulatorMode mode;

    int cpu_hz;
    int cpu_trace;
    int vf_reset;
    int memory_quirk;
    int clip_quirk;
    int shift_quirk;
    int jump_quirk;
    int disp_wait;
    int window_scale;
};

void apply_mode_config(struct Config *conf);
void print_config(struct Config *conf, const char *rom_path, long rom_size);

#endif
