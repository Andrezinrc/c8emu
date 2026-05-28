#include <stdio.h>
#include "config.h"

void print_config(struct Config *conf) {
    printf("CPU CLOCK   : %d Hz\n", conf->cpu_hz * 100);
    printf("VF RESET    : %s\n", conf->vf_reset ? "ON" : "OFF");
    printf("MEM QUIRK   : %s\n", conf->memory_quirk ? "ON" : "OFF");
    printf("DISP.WAIT   : %s\n", conf->disp_wait ? "ON" : "OFF");;
    printf("CLIP QUIRK  : %s\n", conf->clip_quirk ? "ON": "OFF");
    printf("SHIFT QUIRK : %s\n", conf->shift_quirk ? "ON" : "OFF");
    printf("WINDOW SCALE: %dx (%dx%d)\n", conf->window_scale, 64 * conf->window_scale, 32 * conf->window_scale);
    printf("COLOR BG    : 0x%08X\n", conf->bg_color);
    printf("COLOR FG    : 0x%08X\n", conf->fg_color);
}
