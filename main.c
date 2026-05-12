#include "cpu.h"
#include <stdio.h>

int main() {
    struct Chip8 cpu;

    printf("--- Initializing CHIP-8 Emulator ---\n");
    cpu_init(&cpu);

    cpu.ram[0x200] = 0x23;
    cpu.ram[0x201] = 0x00;

    cpu.ram[0x300] = 0x12;
    cpu.ram[0x301] = 0x00;

    printf("\n--- Executing Cycle ---\n");
    while (1) {
        uint16_t op = (cpu.ram[cpu.PC] << 8) | cpu.ram[cpu.PC + 1];
        if (op == 0x0000) {
            break;
        }

        cpu_step(&cpu);
    }

    printf("\nPC after step: 0x%04X\n", cpu.PC);

    return 0;

}
