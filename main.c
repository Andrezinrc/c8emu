#include "cpu.h"
#include <stdio.h>

int main() {
    struct Chip8 cpu;

    printf("--- Initializing CHIP-8 Emulator ---\n");
    cpu_init(&cpu);

    //cpu.ram[0x200] = 0x61; cpu.ram[0x201] = 0x05;
    //cpu.ram[0x202] = 0x71; cpu.ram[0x203] = 0x02;
    //cpu.ram[0x204] = 0x31; cpu.ram[0x205] = 0x07;
    //cpu.ram[0x206] = 0x12; cpu.ram[0x207] = 0x06;
    //cpu.ram[0x208] = 0x12; cpu.ram[0x209] = 0x08;

    //printf("--- STARTING CPU TEST (0x3000 - 0x7000) ---\n");

    /* Step 1: Load V1 */
    //cpu_step(&cpu);
    //printf("[STEP 1] LD V1, 0x05  -> V1: 0x%02X | PC: 0x%03X\n", cpu.V[1], cpu.PC);

    /* Step 2: Add to V1 */
    //cpu_step(&cpu);
    //printf("[STEP 2] ADD V1, 0x02 -> V1: 0x%02X | PC: 0x%03X\n", cpu.V[1], cpu.PC);

    /* Step 3: Conditional Skip */
    //cpu_step(&cpu);
    //printf("[STEP 3] SE V1, 0x07  -> PC: 0x%03X\n\n", cpu.PC);

    //if (cpu.PC == 0x208) {
    //    printf("STATUS: SUCCESS\n");
    //} else {
    //    printf("STATUS: FAILED\n");
    //}

    /*
    printf("\n--- Executing Cycle ---\n");
    while (1) {
        uint16_t op = (cpu.ram[cpu.PC] << 8) | cpu.ram[cpu.PC + 1];
        if (op == 0x0000) {
            break;
        }
        cpu_step(&cpu);
    }
    */
    printf("\nPC after step: 0x%04X\n", cpu.PC);

    return 0;

}
