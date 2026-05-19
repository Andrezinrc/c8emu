#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "video.h"

struct Chip8 {
    uint8_t  memory[4096];
    uint8_t  V[16];
    uint16_t I;
    uint16_t PC;    
    uint16_t stack[16];
    uint8_t  SP;
    
    uint8_t  DT;
    uint8_t  ST;
    
    uint8_t  VIDEO[2048];
    uint8_t  KEYPAD[16];
    int      draw_flag;
};

void cpu_init(struct Chip8 *cpu);
int cpu_load_rom(struct Chip8 *cpu, const char *rom_path);
void cpu_step(struct Chip8 *cpu);
void cpu_update_timers(struct Chip8 *cpu, SDL_AudioDeviceID dev, int8_t *som_buffer);

#endif
