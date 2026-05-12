#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t  ram[4096];
    uint8_t  V[16];
    uint16_t I;
    uint16_t PC;    
    uint16_t stack[16];
    uint8_t  SP;
    uint8_t  delay_timer;
    uint8_t  sound_timer;
    uint8_t  key[16];
    uint8_t  display[64][32];
    bool     draw_flag;
} Chip8;

#endif
