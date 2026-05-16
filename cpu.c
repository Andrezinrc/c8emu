/* @file cpu.c
 *
 * @author André Moreira.
 *
 */
#include "cpu.h"
#include "cpu_internal.h"
#include "display.h"
#include "trace.h"

uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void cpu_init(struct Chip8 *cpu) {
    printf("--- Initializing CHIP-8 Emulator ---\n");

    memset(cpu, 0, sizeof(struct Chip8));
    cpu->PC = 0x200;

    memcpy(&cpu->ram[0], chip8_fontset, sizeof(chip8_fontset));
}

int cpu_load_rom(struct Chip8 *cpu, const char *rom_path) {
    FILE *file = fopen(rom_path, "rb");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    long rom_size = ftell(file);
    rewind(file);
    if (rom_size > (RAM_SIZE - 0x200)) {
        fclose(file);
        return 0;
    }
    size_t bytes_read = fread(&cpu->ram[0x200], 1, rom_size, file);
    fclose(file);

    return bytes_read == (size_t)rom_size;
}

// CPU Instructions

void cpu_step(struct Chip8 *cpu) {
    uint16_t op = (cpu->ram[cpu->PC] << 8) | cpu->ram[cpu->PC + 1];

    /* Decode & Execute */
    switch (op & 0xF000) {
        case 0x0000:
            switch (op) {
                case 0x00E0: /* CLS */
                    TRACE_CPU(cpu, "CLS", op);
                    memset(cpu->display, 0, sizeof(cpu->display));
                    cpu->draw_flag = 1;
                    cpu->PC += 2;
                    break;
                case 0x00EE: /* RET */
                    TRACE_CPU(cpu, "RET", op);
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0x1000: /* 1NNN - JP addr */
            TRACE_CPU(cpu, "JP", op);
            cpu->PC = OP_NNN(op);
            break;
        case 0x2000: /* 2NNN - CALL addr */
            TRACE_CPU(cpu, "CALL", op);
            cpu->stack[cpu->SP] = cpu->PC + 2;
            cpu->SP++;
            cpu->PC = OP_NNN(op);
            break;
        case 0x3000: /* 3XKK - SE Vx, byte */
            TRACE_CPU(cpu, "SE_BYTE", op);
            if (cpu->V[OP_X(op)] == OP_KK(op)) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0x4000: /* 4XKK - SNE Vx, byte */
            TRACE_CPU(cpu, "SNE_BYTE", op);
            if (cpu->V[OP_X(op)] != OP_KK(op)) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0x5000: /* 5XY0 - SE Vx, Vy */
            TRACE_CPU(cpu, "SE_REG", op);
            if (cpu->V[OP_X(op)] == cpu->V[OP_Y(op)]) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0x6000: /* 6XKK - LD Vx, byte */
            TRACE_CPU(cpu, "LD_BYTE", op);
            cpu->V[OP_X(op)] = OP_KK(op);
            cpu->PC += 2;
            break;
        case 0x7000: /* 7XKK - ADD Vx, byte */
            TRACE_CPU(cpu, "ADD_BYTE", op);
            cpu->V[OP_X(op)] += OP_KK(op);
            cpu->PC += 2;
            break;
        case 0x8000:
            switch (OP_N(op)) {
                /* 8XY0 - LD Vx, Vy */
                case 0x0: TRACE_CPU(cpu, "LD_REG", op);  cpu->V[OP_X(op)] = cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY1 - OR Vx, Vy */
                case 0x1: TRACE_CPU(cpu, "OR", op); cpu->V[OP_X(op)] |= cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY2 - AND Vx, Vy */
                case 0x2: TRACE_CPU(cpu, "AND", op); cpu->V[OP_X(op)] &= cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY3 - XOR Vx, Vy */
                case 0x3: TRACE_CPU(cpu, "XOR", op); cpu->V[OP_X(op)] ^= cpu->V[OP_Y(op)]; cpu->PC += 2; break;
                /* 8XY4 - ADD Vx, Vy */
                case 0x4: {
                    TRACE_CPU(cpu, "ADD_REG", op);
                    uint16_t sum = cpu->V[OP_X(op)] + cpu->V[OP_Y(op)];
                    cpu->V[0xF] = (sum > 255) ? 1 : 0;
                    cpu->V[OP_X(op)] = sum & 0xFF;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY5 - SUB Vx, Vy */
                case 0x5: {
                    TRACE_CPU(cpu, "SUB", op);
                    uint8_t borrow = cpu->V[OP_X(op)] > cpu->V[OP_Y(op)] ? 1 : 0;
                    cpu->V[OP_X(op)] -= cpu->V[OP_Y(op)];
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY6 - SHR Vx {, Vy} */
                case 0x6:
                    TRACE_CPU(cpu, "SHR", op);
                    cpu->V[0xF] = cpu->V[OP_X(op)] & 0x01;
                    cpu->V[OP_X(op)] >>= 1;
                    cpu->PC += 2;
                    break;
                /* 8XY7 - SUBN Vx, Vy */
                case 0x7: {
                    TRACE_CPU(cpu, "SUBN", op);
                    uint16_t sub = cpu->V[OP_Y(op)] - cpu->V[OP_X(op)];
                    uint8_t borrow = cpu->V[OP_Y(op)] > cpu->V[OP_X(op)] ? 1 : 0;
                    cpu->V[OP_X(op)] = sub;
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                /* 8XYE - SHL Vx {, Vy} */
                case 0xE:
                    TRACE_CPU(cpu, "SHL", op);
                    cpu->V[0xF] = (cpu->V[OP_X(op)] >> 7) & 0x01;
                    cpu->V[OP_X(op)] <<= 1;
                    cpu->PC += 2;
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0x9000: /* 9XY0 - SNE Vx, Vy */
            TRACE_CPU(cpu, "SNE_REG", op);
            if (cpu->V[OP_X(op)] != cpu->V[OP_Y(op)]) cpu->PC += 4;
            else cpu->PC += 2;
            break;
        case 0xA000: /* ANNN - LD I, addr */
            TRACE_CPU(cpu, "LD_I", op);
            cpu->I = OP_NNN(op);
            cpu->PC += 2;
            break;
        case 0xB000: /* BNNN - JP V0, addr */
            TRACE_CPU(cpu, "JP_V0", op);
            cpu->PC = OP_NNN(op) + cpu->V[0];
            break;
        case 0xC000: /* CXKK - RND Vx, byte */
            TRACE_CPU(cpu, "RND", op);
            cpu->V[OP_X(op)] = (rand() % 256) & OP_KK(op);
            cpu->PC += 2;
            break;
        case 0xD000: { /* DXYN - DRW Vx, Vy, nibble */
            TRACE_CPU(cpu, "DRW", op);
            uint8_t x = cpu->V[OP_X(op)];
            uint8_t y = cpu->V[OP_Y(op)];
            uint8_t height = OP_N(op);

            cpu->V[0xF] = 0;

            for (int row = 0; row < height; row++) {
                uint8_t sprite_byte =  cpu->ram[cpu->I + row];
                for (int col = 0; col < 8; col++) {
                    uint8_t bit = (sprite_byte & (0x80 >> col)) ? 1 : 0;
                    
                    int pixel_x = (x + col) % 64;
                    int pixel_y = (y + row) % 32;

                    if (bit == 1) {
                        if (cpu->display[pixel_x][pixel_y] == 1) {
                            cpu->V[0xF] = 1;
                        }
                        cpu->display[pixel_x][pixel_y] ^= 1;
                    }
                }
            }

            cpu->draw_flag = 1;
            cpu->PC += 2;
            break;
        }
        case 0xE000:
            switch (OP_KK(op)) {
                case 0x009E: /* EX9E - SKP Vx */
                    TRACE_CPU(cpu, "SKP", op);
                    if (cpu->key[cpu->V[OP_X(op)]] != 0) cpu->PC += 4;
                    else cpu->PC += 2;
                    break;
                case 0x00A1: /* EXA1 - SKNP Vx */
                    TRACE_CPU(cpu, "SKNP", op);
                    if (cpu->key[cpu->V[OP_X(op)]] == 0) cpu->PC += 4;
                    else cpu->PC += 2;
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        default:
            UNKNOWN_OPCODE(op);
    }
}

void cpu_update_timers(struct Chip8 *cpu) {
    if (cpu->delay_timer > 0) {
        cpu->delay_timer--;
    }
    if (cpu->sound_timer > 0) {
        cpu->sound_timer--;
    }
}

void cpu_cycles(struct Chip8 *cpu, SDL_Renderer *ren, long cycles) {
#ifdef DEBUG_MODE    
    printf("\n--- Executing Cycle ---\n");
#endif
    while (cycles > 0) {
        cpu_step(cpu);
        cycles--;
    }

    if (cpu->draw_flag) {
        display_update(ren, cpu);
        cpu->draw_flag = 0;
    }
}
