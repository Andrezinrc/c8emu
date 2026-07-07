#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "SDL/video.h"
#include "config.h"

struct Chip8 {
    uint8_t  memory[65536]; 
    uint8_t  V[16];
    uint16_t I;
    uint16_t PC;
    uint16_t stack[16];
    uint8_t  SP;

    uint8_t  DT;
    uint8_t  ST;

    uint8_t  VIDEO[16384]; 
    uint8_t  KEYPAD[16];
    int      draw_flag;
    int      disp_wait;

    uint8_t  selected_plane; 
    uint8_t  audio_pattern[16];
    uint8_t  audio_pitch;
    uint8_t  RPL[16]; 
    int      hires_mode; 
};

void cpu_init(struct Chip8 *cpu);
int cpu_load_rom(struct Chip8 *cpu, const char *rom_path);
void cpu_step(struct Chip8 *cpu, struct Config *conf);
void cpu_update_timers(struct Chip8 *cpu, SDL_AudioDeviceID dev, int8_t *som_buffer);

#endif
