#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#define RAM_SIZE   4096
#define REG_COUNT  16
#define STACK_SIZE 16
#define KEY_COUNT  16

#define OP_KK(op)  ((op) & 0x00FF)

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
    bool     draw_flag;
};

void cpu_init(struct Chip8 *cpu);
void cpu_step(struct Chip8 *cpu);

#endif
