#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "display.h"

#define RAM_SIZE   4096
#define REG_COUNT  16
#define STACK_SIZE 16
#define KEY_COUNT  16

struct Chip8 {
    uint8_t  ram[RAM_SIZE];
    uint8_t  V[REG_COUNT];
    uint16_t I;
    uint16_t PC;    
    uint16_t stack[STACK_SIZE];
    uint8_t  SP;
    uint8_t  delay_timer;
    uint8_t  sound_timer;
    uint8_t  key[KEY_COUNT];
    uint8_t  display[64][32];
    int      draw_flag;
};

extern void (*cpu_op_handler[0x10000])(struct Chip8*, uint16_t);
extern const char* cpu_op_name[0x10000];

int cpu_load_rom(struct Chip8 *cpu, const char *rom_path);
void cpu_init(struct Chip8 *cpu);
void cpu_step(struct Chip8 *cpu);
void cpu_update_timers(struct Chip8 *cpu);
void cpu_cycles(struct Chip8 *cpu, SDL_Renderer *ren, long cycles);

#endif
