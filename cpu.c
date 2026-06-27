#include "cpu_internal.h"

void cpu_init(struct Chip8 *cpu)
{
    printf("--- Initializing CHIP-8 Emulator ---\n");

    memset(cpu, 0, sizeof(struct Chip8));
    cpu->PC = 0x200;

    memcpy(&cpu->memory[0], chip8_fontset, sizeof(chip8_fontset));
}

int cpu_load_rom(struct Chip8 *cpu, const char *rom_path)
{
    FILE *file = fopen(rom_path, "rb");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    long rom_size = ftell(file);
    rewind(file);

    size_t bytes_read = fread(&cpu->memory[0x200], 1, rom_size, file);
    fclose(file);

    return bytes_read == (size_t)rom_size;
}

// CPU Instructions

void cpu_step(struct Chip8 *cpu, struct Config *conf)
{
    if (!cpu->disp_wait) return;

    uint16_t op = (cpu->memory[cpu->PC] << 8) | cpu->memory[cpu->PC + 1];

    /* Decode & Execute */
    switch (op & 0xF000) {
        case 0x0000:
            switch (OP_KK(op)) {
                case 0x00E0: /* CLS */
                    TRACE_CPU(cpu, "CLS", op);
                    memset(cpu->VIDEO, 0, sizeof(cpu->VIDEO));
                    cpu->draw_flag = 1;
                    cpu->PC += 2;
                    break;
                case 0x00EE: /* RET */
                    TRACE_CPU(cpu, "RET", op);
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                default: /* SYS addr */
                    TRACE_CPU(cpu, "SYS", op);
                    cpu->PC += 2;
                    break;
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
            if (cpu->V[OP_X(op)] == OP_KK(op))
                cpu->PC += 4;
            else
                cpu->PC += 2;
            break;
        case 0x4000: /* 4XKK - SNE Vx, byte */
            TRACE_CPU(cpu, "SNE_BYTE", op);
            if (cpu->V[OP_X(op)] != OP_KK(op))
                cpu->PC += 4;
            else
                cpu->PC += 2;
            break;
        case 0x5000: /* 5XY0 - SE Vx, Vy */
            TRACE_CPU(cpu, "SE_REG", op);
            if (cpu->V[OP_X(op)] == cpu->V[OP_Y(op)])
                cpu->PC += 4;
            else
                cpu->PC += 2;
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
                case 0x0:
                    TRACE_CPU(cpu, "LD_REG", op);
                    cpu->V[OP_X(op)] = cpu->V[OP_Y(op)];
                    cpu->PC += 2;
                    break;
                /* 8XY1 - OR Vx, Vy */
                case 0x1:
                    TRACE_CPU(cpu, "OR", op);
                    cpu->V[OP_X(op)] |= cpu->V[OP_Y(op)];
                    if (conf->vf_reset)
                        cpu->V[0xF] = 0;
                    cpu->PC += 2;
                    break;
                /* 8XY2 - AND Vx, Vy */
                case 0x2:
                    TRACE_CPU(cpu, "AND", op);
                    cpu->V[OP_X(op)] &= cpu->V[OP_Y(op)];
                    if (conf->vf_reset)
                        cpu->V[0xF] = 0;
                    cpu->PC += 2;
                    break;
                /* 8XY3 - XOR Vx, Vy */
                case 0x3:
                    TRACE_CPU(cpu, "XOR", op);
                    cpu->V[OP_X(op)] ^= cpu->V[OP_Y(op)];
                    if (conf->vf_reset)
                        cpu->V[0xF] = 0;
                    cpu->PC += 2;
                    break;
                /* 8XY4 - ADD Vx, Vy */
                case 0x4: {
                    TRACE_CPU(cpu, "ADD_REG", op);
                    uint16_t sum = cpu->V[OP_X(op)] + cpu->V[OP_Y(op)];
                    cpu->V[OP_X(op)] = sum & 0xFF;
                    cpu->V[0xF] = (sum > 255) ? 1 : 0;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY5 - SUB Vx, Vy */
                case 0x5: {
                    TRACE_CPU(cpu, "SUB", op);
                    uint8_t borrow = (cpu->V[OP_X(op)] >= cpu->V[OP_Y(op)]) ? 1 : 0;
                    cpu->V[OP_X(op)] -= cpu->V[OP_Y(op)];
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY6 - SHR Vx {, Vy} */
                case 0x6: {
                    TRACE_CPU(cpu, "SHR", op);
                    uint8_t target_val = conf->shift_quirk ? cpu->V[OP_X(op)] : cpu->V[OP_Y(op)];
                    uint8_t f_val = target_val & 0x01;
                    cpu->V[OP_X(op)] = target_val >> 1;
                    cpu->V[0xF] = f_val;
                    cpu->PC += 2;
                    break;
                }
                /* 8XY7 - SUBN Vx, Vy */
                case 0x7: {
                    TRACE_CPU(cpu, "SUBN", op);
                    uint16_t sub = cpu->V[OP_Y(op)] - cpu->V[OP_X(op)];
                    uint8_t borrow = (cpu->V[OP_Y(op)] >= cpu->V[OP_X(op)]) ? 1 : 0;
                    cpu->V[OP_X(op)] = sub;
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                /* 8XYE - SHL Vx {, Vy} */
                case 0xE: {
                    TRACE_CPU(cpu, "SHL", op);
                    uint8_t target_val = conf->shift_quirk ? cpu->V[OP_X(op)] : cpu->V[OP_Y(op)];
                    uint8_t f_val = (target_val >> 7) & 0x01;
                    cpu->V[OP_X(op)] = target_val << 1;
                    cpu->V[0xF] = f_val;
                    cpu->PC += 2;
                    break;
                }
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0x9000: /* 9XY0 - SNE Vx, Vy */
            TRACE_CPU(cpu, "SNE_REG", op);
            if (cpu->V[OP_X(op)] != cpu->V[OP_Y(op)])
                cpu->PC += 4;
            else
                cpu->PC += 2;
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
            uint8_t x = cpu->V[OP_X(op)] % 64;
            uint8_t y = cpu->V[OP_Y(op)] % 32;
            uint8_t height = OP_N(op);

            cpu->V[0xF] = 0;

            for (int row = 0; row < height; row++) {

                uint8_t sprite_byte =  cpu->memory[cpu->I + row];
                for (int col = 0; col < 8; col++) {
                    
                    if ((sprite_byte & (0x80 >> col)) != 0) {
                        int px = x + col;
                        int py = y + row;

                        if (!conf->clip_quirk) {
                            px %= 64;
                            py %= 32;
                        } else {
                            if (px >= 64 || py >= 32)
                                continue;
                        }

                        int vid_index = px + (py * 64);

                        if (cpu->VIDEO[vid_index] == 1)
                            cpu->V[0xF] = 1;

                        cpu->VIDEO[vid_index] ^= 1;
                    }
                }
            }

            cpu->draw_flag = 1;
            if (conf->disp_wait)
                cpu->disp_wait = 0;
            cpu->PC += 2;
            break;
        }
        case 0xE000:
            switch (OP_KK(op)) {
                case 0x009E: /* EX9E - SKP Vx */
                    TRACE_CPU(cpu, "SKP", op);
                    if (cpu->KEYPAD[cpu->V[OP_X(op)]] != 0)
                        cpu->PC += 4;
                    else
                        cpu->PC += 2;
                    break;
                case 0x00A1: /* EXA1 - SKNP Vx */
                    TRACE_CPU(cpu, "SKNP", op);
                    if (cpu->KEYPAD[cpu->V[OP_X(op)]] == 0)
                        cpu->PC += 4;
                    else
                        cpu->PC += 2;
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0xF000:
            switch (OP_KK(op)) {
                case 0x0007: /* FX07 - LD Vx, DT */
                    TRACE_CPU(cpu, "LD_VX_DT", op);
                    cpu->V[OP_X(op)] = cpu->DT;
                    cpu->PC += 2;
                    break;
                case 0x000A: { /* FX0A - LD Vx, K */
                    TRACE_CPU(cpu, "LD_VX_K", op);
                    static int waiting_for_release = 0;
                    static int saved_key = -1;

                    if (waiting_for_release) {
                        if (cpu->KEYPAD[saved_key] == 0) {
                            cpu->V[OP_X(op)] = saved_key;
                            waiting_for_release = 0;
                            saved_key = -1;
                            cpu->PC += 2;
                        }
                        return;
                    }
                    for (int i = 0; i < 16; i++)
                        if (cpu->KEYPAD[i] != 0) {
                            saved_key = i;
                            waiting_for_release = 1;
                            return;
                        }

                    return;
                }
                case 0x0015: /* FX15 - LD DT, Vx */
                    TRACE_CPU(cpu, "LD_DT_VX", op);
                    cpu->DT = cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x0018: /* FX18 - LD ST, Vx */
                    TRACE_CPU(cpu, "LD_ST_VX", op);
                    cpu->ST =  cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x001E: /* FX1E - ADD I, Vx */
                    TRACE_CPU(cpu, "ADD_I_VX", op);
                    cpu->I += cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x0029: /* FX29 - LD F, Vx */
                    TRACE_CPU(cpu, "LD_F_VX", op);
                    cpu->I = cpu->V[OP_X(op)] * 5;
                    cpu->PC += 2;
                    break;
                case 0x0033: { /* FX33 - LD B, Vx */
                    TRACE_CPU(cpu, "LD_B_VX", op);
                    uint8_t value = cpu->V[OP_X(op)];
                    cpu->memory[cpu->I] = value / 100;
                    cpu->memory[cpu->I + 1] = (value / 10) % 10;
                    cpu->memory[cpu->I + 2] = value % 10;
                    cpu->PC += 2;
                    break;
                }
                case 0x0055: { /* FX55 - LD [I], Vx */
                    TRACE_CPU(cpu, "LD_MEM_VX", op);
                    int x = OP_X(op);
                    for (int i = 0; i <= x; i++)
                        cpu->memory[cpu->I + i] = cpu->V[i];

                    if (conf->memory_quirk)
                        cpu->I += x + 1;

                    cpu->PC += 2;
                    break;
                }
                case 0x0065: { /* FX65 - LD Vx, [I] */
                    TRACE_CPU(cpu, "LD_VX_MEM", op);
                    int x = OP_X(op);
                    for (int i = 0; i <= x; i++)
                        cpu->V[i] = cpu->memory[cpu->I + i];

                    if (conf->memory_quirk)
                        cpu->I += x + 1;

                    cpu->PC += 2;
                    break;
                }
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        default:
            UNKNOWN_OPCODE(op);
    }
}

void cpu_update_timers(struct Chip8 *cpu, SDL_AudioDeviceID aud_dev,
                            int8_t *som_buffer)
{
    if (cpu->DT > 0)
        cpu->DT--;
    if (cpu->ST > 0) {
        SDL_PauseAudioDevice(aud_dev, 0);
        SDL_QueueAudio(aud_dev, som_buffer, 44100 / 60);        
        cpu->ST--;
    }
    else {
        SDL_PauseAudioDevice(aud_dev, 1);
        SDL_ClearQueuedAudio(aud_dev);
    }
}
