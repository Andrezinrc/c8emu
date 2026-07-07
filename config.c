#include <stdio.h>
#include "config.h"

void chip8_mode(struct Config *conf)
{
    conf->cpu_hz = 8; // 800Hz
    conf->memory_quirk = 1;
    conf->shift_quirk = 0;
    conf->jump_quirk = 0;
    conf->clip_quirk = 1;
    conf->vf_reset = 1;
    conf->disp_wait = 1;
}

void schip_mode(struct Config *conf)
{
    conf->cpu_hz = 15; // 1500Hz
    conf->memory_quirk = 0;
    conf->shift_quirk = 1;
    conf->jump_quirk = 1;
    conf->clip_quirk = 1;
    conf->vf_reset = 0;
    conf->disp_wait = 1;
}

void xochip_mode(struct Config *conf)
{
    conf->cpu_hz = 150; // 15000Hz
    conf->memory_quirk = 1;
    conf->shift_quirk = 0;
    conf->jump_quirk = 0;
    conf->clip_quirk = 0;
    conf->vf_reset = 0;
    conf->disp_wait = 0;
}

void apply_mode_config(struct Config *conf)
{
   switch (conf->mode) {
        case MODE_CHIP8:
            chip8_mode(conf);
            break;
        case MODE_SCHIP:
            schip_mode(conf);
            break;
        case MODE_XOCHIP:
            xochip_mode(conf);
            break;
    }
}

void print_config(struct Config *conf, const char *rom_path,
                                long rom_size)
{
    printf("[ROM] Loading: %s (%ld bytes)\n", rom_path, rom_size);
    printf("[ROM] Mode: %s\n", conf->mode == MODE_CHIP8 ? "CHIP-8" :
                    (conf->mode == MODE_SCHIP ? "SCHIP" : "XO-CHIP"));
    printf("[CPU] Clock %d Hz\n", conf->cpu_hz * 100);
    printf("[VIDEO] Display initialized at %s (Scale: %dx)\n",
                    conf->mode == MODE_CHIP8 ? "64x32" : "128x64", conf->window_scale);
    printf("[CPU] Booting... PC = 0x200\n");
}
