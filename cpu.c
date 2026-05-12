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
                default:
                    fprintf(stderr, "Unknown 0x0000: 0x%04X\n", op);
                    exit(1);
                    break;
            }
            break;

        default:
            fprintf(stderr, "Opcode 0x%04X not implemented yet!\n", op);
            exit(1); 
            break;
    }
}
