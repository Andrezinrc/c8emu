#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define UNKNOWN_OPCODE(op) \
    do { \
        fprintf(stderr, "Unknown opcode: 0x%04X at PC: 0x%03X\n", \
                op, cpu->PC); \
        exit(1); \
    } while (0)


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
            switch (op) {
                case 0x00E0: /* CLS */
                    memset(cpu->display, 0, sizeof(cpu->display));
                    cpu->draw_flag = true;
                    cpu->PC += 2;
                    break;
                case 0x00EE: /* RET */
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                default:
                    UNKNOWN_OPCODE(op);
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
        case 0x8000:
            switch (OP_N(op)) {
                /* 8XY0 - LD Vx, Vy */
                case 0x0: cpu->V[OP_X(op)] = cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY1 - OR Vx, Vy */
                case 0x1: cpu->V[OP_X(op)] |= cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY2 - AND Vx, Vy */
                case 0x2: cpu->V[OP_X(op)] &= cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY3 - XOR Vx, Vy */
                case 0x3: cpu->V[OP_X(op)] ^= cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY4 - ADD Vx, Vy */
                case 0x4: {
                    uint16_t sum = cpu->V[OP_X(op)] + cpu->V[OP_Y(op)];
                    cpu->V[0xF] = (sum > 255) ? 1 : 0;
                    cpu->V[OP_X(op)] = sum & 0xFF;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY5 - SUB Vx, Vy */
                case 0x5: {
                    uint8_t borrow = cpu->V[OP_X(op)] > cpu->V[OP_Y(op)] ? 1 : 0;
                    cpu->V[OP_X(op)] -= cpu->V[OP_Y(op)];
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY6 - SHR Vx {, Vy} */
                case 0x6:
                    cpu->V[0xF] = cpu->V[OP_X(op)] & 0x01;
                    cpu->V[OP_X(op)] >>= 1;
                    cpu->PC += 2;
                    break;
                /* 8XY7 - SUBN Vx, Vy */
                case 0x7: {
                    uint16_t sub = cpu->V[OP_Y(op)] - cpu->V[OP_X(op)];
                    uint8_t borrow = cpu->V[OP_Y(op)] > cpu->V[OP_X(op)] ? 1 : 0;
                    cpu->V[OP_X(op)] = sub;
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                /* 8XYE - SHL Vx {, Vy} */
                case 0xE:
                    cpu->V[0xF] = (cpu->V[OP_X(op)] >> 7) & 0x01;
                    cpu->V[OP_X(op)] <<= 1;
                    cpu->PC += 2;
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0x9000: /* 9XY0 - SNE Vx, Vy */
            if (cpu->V[OP_X(op)] != cpu->V[OP_Y(op)]) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0xA000: /* ANNN - LD I, addr */
            cpu->I = OP_NNN(op);
            cpu->PC += 2;
            break;
        case 0xB000: /* BNNN - JP V0, addr */
            cpu->PC = OP_NNN(op) + cpu->V[0];
            break;
        case 0xC000: /* CXKK - RND Vx, byte */
            cpu->V[OP_X(op)] = (rand() % 256) & OP_KK(op);
            cpu->PC += 2;
            break;
        default:
            UNKNOWN_OPCODE(op);
    }
}
