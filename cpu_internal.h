#ifndef CPU_INTERNAL_H
#define CPU_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

#define UNKNOWN_OPCODE(op) \
    do { \
        fprintf(stderr, "Unknown opcode: 0x%04X at PC: 0x%03X\n", op, cpu->PC); \
        exit(1); \
    } while (0)

#define OP_X(op)   (((op) & 0x0F00) >> 8)
#define OP_Y(op)   (((op) & 0x00F0) >> 4)
#define OP_KK(op)  ((op)  & 0x00FF)
#define OP_N(op)   ((op)  & 0x000F)
#define OP_NNN(op) ((op)  & 0x0FFF)

#endif
