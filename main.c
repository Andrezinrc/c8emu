#include "cpu.h"
#include <stdio.h>

int main() {
    struct Chip8 cpu;

    printf("--- Initializing CHIP-8 Emulator ---\n");
    cpu_init(&cpu);

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
