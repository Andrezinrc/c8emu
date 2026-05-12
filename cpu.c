#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* André Moreira - 05/2026 */

void cpu_init(struct Chip8 *cpu) {
    memset(cpu, 0, sizeof(struct Chip8));
    cpu->PC = 0x200;
}

void cpu_step(struct Chip8 *cpu) {
    uint16_t op = (cpu->ram[cpu->PC] << 8) | cpu->ram[cpu->PC + 1];
    
    printf("PC: 0x%04X | Opcode: 0x%04X\n", cpu->PC, op);
    
    /* Decode & Execute */
    switch (op & 0xF000) {
        case 0x0000:
            switch (OP_KK(op)) {
                case 0x00E0: /* CLS */
                    memset(cpu->display, 0, sizeof(cpu->display));
                    cpu->draw_flag = true;
                    cpu->PC += 2;
                    break;
                case 0x00EE: /* RET */
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                default:     /* 0NNN - SYS addr */
                    cpu->PC += 2;
                    break;
            }
            break;
        case 0x1000: /* 1NNN - JP addr */
            cpu->PC = OP_NNN(op);
            break;
        case 0x2000: /* 2NNN - CALL addr */
            cpu->stack[cpu->SP] = cpu->PC + 2;
            cpu->SP++;
            cpu->PC = OP_NNN(op);
            break;
        case 0x3000: /* 3XKK - SE Vx, byte */
            if (cpu->V[OP_X(op)] == OP_KK(op)) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0x4000: /* 4XKK - SNE Vx, byte */
            if (cpu->V[OP_X(op)] != OP_KK(op)) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0x5000: /* 5XY0 - SE Vx, Vy */
            if (cpu->V[OP_X(op)] == cpu->V[OP_Y(op)]) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0x6000: /* 6XKK - LD Vx, byte */
            cpu->V[OP_X(op)] = OP_KK(op);
            cpu->PC += 2;
            break;
        case 0x7000: /* 7XKK - ADD Vx, byte */
            cpu->V[OP_X(op)] += OP_KK(op);
            cpu->PC += 2;
            break;
        default:
            fprintf(stderr, "Opcode 0x%04X not implemented yet!\n", op);
            exit(1); 
            break;
    }
}
