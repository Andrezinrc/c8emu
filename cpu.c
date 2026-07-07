#include "cpu_internal.h"
#include <stdlib.h>
#include <string.h>

void cpu_init(struct Chip8 *cpu)
{
    //printf("--- Initializing CHIP-8 Emulator ---\n");
    
    memset(cpu, 0, sizeof(struct Chip8));
    
    cpu->PC = 0x200;
    cpu->selected_plane = 1;
    cpu->audio_pitch = 64;
    cpu->hires_mode = 0;

    memcpy(&cpu->memory[0], chip8_fontset, sizeof(chip8_fontset));
    memcpy(&cpu->memory[0x50], schip_fontset, sizeof(schip_fontset));
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

static void skip_instruction(struct Chip8 *cpu)
{
    uint16_t next_op = (cpu->memory[cpu->PC] << 8) | cpu->memory[cpu->PC + 1];
    
    // Xochip uses 4 byte wide instructions for long
    // loads so we need double skip step 
    if ((next_op & 0xFFFF) == 0xF000)
        cpu->PC += 4;
    else
        cpu->PC += 2;
}

void cpu_step(struct Chip8 *cpu, struct Config *conf)
{
    if (!cpu->disp_wait) return;

    uint16_t op = (cpu->memory[cpu->PC] << 8) | cpu->memory[cpu->PC + 1];

    switch (op & 0xF000) {
        case 0x0000: {
            int width = cpu->hires_mode ? 128 : 64;
            int height = cpu->hires_mode ? 64 : 32;

            if ((op & 0xFFF0) == 0x00C0) { /* 00CN - Scroll Down N */
                TRACE_CPU(conf, cpu, "SCD", op);
                uint8_t n = op & 0x000F;
                for (int plane = 0; plane < 2; plane++) {
                    if (!(cpu->selected_plane & (1 << plane)))
                        continue;
                    // Work backwards to avoid overwriting rows we still need to read
                    uint8_t *p_vid = &cpu->VIDEO[plane * 8192];
                    for (int y = height - 1; y >= n; y--)
                        memmove(&p_vid[y * width], &p_vid[(y - n) * width], width);
                    for (int y = 0; y < n; y++)
                        memset(&p_vid[y * width], 0, width);
                }
                cpu->draw_flag = 1;
                cpu->PC += 2;
                break;
            } 
            else if ((op & 0xFFF0) == 0x00D0) { /* 00DN - Scroll Up N */
                TRACE_CPU(conf, cpu, "SCU", op);
                uint8_t n = op & 0x000F;
                for (int plane = 0; plane < 2; plane++) {
                    if (!(cpu->selected_plane & (1 << plane)))
                        continue;
                    uint8_t *p_vid = &cpu->VIDEO[plane * 8192];
                    for (int y = 0; y < height - n; y++)
                        memmove(&p_vid[y * width], &p_vid[(y + n) * width], width);
                    for (int y = height - n; y < height; y++)
                        memset(&p_vid[y * width], 0, width);
                }
                cpu->draw_flag = 1;
                cpu->PC += 2;
                break;
            }
            switch (OP_KK(op)) {
                case 0x00E0: /* CLS */
                    TRACE_CPU(conf, cpu, "CLS", op);
                    int total_pixels = cpu->hires_mode ? 8192 : 2048;
                    for (int plane = 0; plane < 2; plane++)
                        if (cpu->selected_plane & (1 << plane))
                            memset(&cpu->VIDEO[plane * 8192], 0, total_pixels);
                    cpu->draw_flag = 1;
                    cpu->PC += 2;
                    break;
                case 0x00EE: /* RET */
                    TRACE_CPU(conf, cpu, "RET", op);
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                case 0x00FB: /* Scroll Right 4 */
                    TRACE_CPU(conf, cpu, "SCR", op);
                    for (int plane = 0; plane < 2; plane++) {
                        if (!(cpu->selected_plane & (1 << plane)))
                            continue;
                        uint8_t *p_vid = &cpu->VIDEO[plane * 8192];
                        for (int y = 0; y < height; y++) {
                            memmove(&p_vid[y * width + 4], &p_vid[y * width], width - 4);
                            memset(&p_vid[y * width], 0, 4);
                        }
                    }
                    cpu->draw_flag = 1;
                    cpu->PC += 2;
                    break;
                case 0x00FC: /* Scroll Left 4 */
                    TRACE_CPU(conf, cpu, "SCL", op);
                    for (int plane = 0; plane < 2; plane++) {
                        if (!(cpu->selected_plane & (1 << plane)))
                            continue;
                        uint8_t *p_vid = &cpu->VIDEO[plane * 8192];
                        for (int y = 0; y < height; y++) {
                            memmove(&p_vid[y * width], &p_vid[y * width + 4], width - 4);
                            memset(&p_vid[y * width + (width - 4)], 0, 4);
                        }
                    }
                    cpu->draw_flag = 1;
                    cpu->PC += 2;
                    break;
                case 0x00FD: /* Exit */
                    exit(0);
                    break;
                case 0x00FE: /* LO-RES */
                    TRACE_CPU(conf, cpu, "LORES", op);
                    cpu->hires_mode = 0;
                    cpu->PC += 2;
                    break;
                case 0x00FF: /* HI-RES */
                    TRACE_CPU(conf, cpu, "HIRES", op);
                    cpu->hires_mode = 1;
                    cpu->PC += 2;
                    break;
                default: /* SYS addr */
                    TRACE_CPU(conf, cpu, "SYS", op);
                    cpu->PC += 2;
                    break;
            }
            break;
        }
        case 0x1000: /* 1NNN - JP addr */
            TRACE_CPU(conf, cpu, "JP", op);
            cpu->PC = OP_NNN(op);
            break;
        case 0x2000: /* 2NNN - CALL addr */
            TRACE_CPU(conf, cpu, "CALL", op);
            cpu->stack[cpu->SP] = cpu->PC + 2;
            cpu->SP++;
            cpu->PC = OP_NNN(op);
            break;
        case 0x3000: /* 3XKK - SE Vx, byte */
            TRACE_CPU(conf, cpu, "SE_BYTE", op);
            cpu->PC += 2;
            if (cpu->V[OP_X(op)] == OP_KK(op))
                skip_instruction(cpu);
            break;
        case 0x4000: /* 4XKK - SNE Vx, byte */
            TRACE_CPU(conf, cpu, "SNE_BYTE", op);
            cpu->PC += 2;
            if (cpu->V[OP_X(op)] != OP_KK(op))
                skip_instruction(cpu);
            break;
        case 0x5000:
            switch (OP_N(op)) {
                case 0x0: /* 5XY0 - SE Vx, Vy */
                    TRACE_CPU(conf, cpu, "SE_REG", op);
                    cpu->PC += 2;
                    if (cpu->V[OP_X(op)] == cpu->V[OP_Y(op)])
                        skip_instruction(cpu);
                    break;
                case 0x2: { /* 5XY2 - LD [I], Vx-Vy */
                    TRACE_CPU(conf, cpu, "LD_MEM_V_RANGE", op);
                    int x = OP_X(op);
                    int y = OP_Y(op);
                    // Range can run backwards depending on which register index is bigger
                    int step = (x <= y) ? 1 : -1;
                    int idx = 0;
                    for (int i = x; ; i += step) {
                        cpu->memory[cpu->I + idx] = cpu->V[i];
                        idx++;
                        if (i == y) break;
                    }
                    cpu->PC += 2;
                    break;
                }
                case 0x3: { /* 5XY3 - LD Vx-Vy, [I] */
                    TRACE_CPU(conf, cpu, "LD_V_RANGE_MEM", op);
                    int x = OP_X(op);
                    int y = OP_Y(op);
                    int step = (x <= y) ? 1 : -1;
                    int idx = 0;
                    for (int i = x; ; i += step) {
                        cpu->V[i] = cpu->memory[cpu->I + idx];
                        idx++;
                        if (i == y) break;
                    }
                    cpu->PC += 2;
                    break;
                }
                case 0xD: /* 5XYD - SNE Vx, Vy */
                    TRACE_CPU(conf, cpu, "SNE_REG_XO", op);
                    cpu->PC += 2;
                    if (cpu->V[OP_X(op)] != cpu->V[OP_Y(op)])
                        skip_instruction(cpu);
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0x6000: /* 6XKK - LD Vx, byte */
            TRACE_CPU(conf, cpu, "LD_BYTE", op);
            cpu->V[OP_X(op)] = OP_KK(op);
            cpu->PC += 2;
            break;
        case 0x7000: /* 7XKK - ADD Vx, byte */
            TRACE_CPU(conf, cpu, "ADD_BYTE", op);
            cpu->V[OP_X(op)] += OP_KK(op);
            cpu->PC += 2;
            break;
        case 0x8000:
            switch (OP_N(op)) {
                case 0x0: /* 8XY0 - LD Vx, Vy */
                    TRACE_CPU(conf, cpu, "LD_REG", op);
                    cpu->V[OP_X(op)] = cpu->V[OP_Y(op)];
                    cpu->PC += 2;
                    break;
                case 0x1: /* 8XY1 - OR Vx, Vy */
                    TRACE_CPU(conf, cpu, "OR", op);
                    cpu->V[OP_X(op)] |= cpu->V[OP_Y(op)];
                    if (conf->vf_reset)
                        cpu->V[0xF] = 0;
                    cpu->PC += 2;
                    break;
                case 0x2: /* 8XY2 - AND Vx, Vy */
                    TRACE_CPU(conf, cpu, "AND", op);
                    cpu->V[OP_X(op)] &= cpu->V[OP_Y(op)];
                    if (conf->vf_reset)
                        cpu->V[0xF] = 0;
                    cpu->PC += 2;
                    break;
                case 0x3: /* 8XY3 - XOR Vx, Vy */
                    TRACE_CPU(conf, cpu, "XOR", op);
                    cpu->V[OP_X(op)] ^= cpu->V[OP_Y(op)];
                    if (conf->vf_reset)
                        cpu->V[0xF] = 0;
                    cpu->PC += 2;
                    break;
                case 0x4: { /* 8XY4 - ADD Vx, Vy */
                    TRACE_CPU(conf, cpu, "ADD_REG", op);
                    uint16_t sum = cpu->V[OP_X(op)] + cpu->V[OP_Y(op)];
                    cpu->V[OP_X(op)] = sum & 0xFF;
                    cpu->V[0xF] = (sum > 255) ? 1 : 0;
                    cpu->PC += 2;
                    break;
                }
                case 0x5: { /* 8XY5 - SUB Vx, Vy */
                    TRACE_CPU(conf, cpu, "SUB", op);
                    uint8_t borrow = (cpu->V[OP_X(op)] >= cpu->V[OP_Y(op)]) ? 1 : 0;
                    cpu->V[OP_X(op)] -= cpu->V[OP_Y(op)];
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                case 0x6: { /* 8XY6 - SHR Vx {, Vy} */
                    TRACE_CPU(conf, cpu, "SHR", op);
                    uint8_t target_val = conf->shift_quirk ? cpu->V[OP_X(op)] : cpu->V[OP_Y(op)];
                    uint8_t f_val = target_val & 0x01;
                    cpu->V[OP_X(op)] = target_val >> 1;
                    cpu->V[0xF] = f_val;
                    cpu->PC += 2;
                    break;
                }
                case 0x7: { /* 8XY7 - SUBN Vx, Vy */
                    TRACE_CPU(conf, cpu, "SUBN", op);
                    uint16_t sub = cpu->V[OP_Y(op)] - cpu->V[OP_X(op)];
                    uint8_t borrow = (cpu->V[OP_Y(op)] >= cpu->V[OP_X(op)]) ? 1 : 0;
                    cpu->V[OP_X(op)] = sub;
                    cpu->V[0xF] = borrow;
                    cpu->PC += 2;
                    break;
                }
                case 0xE: { /* 8XYE - SHL Vx {, Vy} */
                    TRACE_CPU(conf, cpu, "SHL", op);
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
            TRACE_CPU(conf, cpu, "SNE_REG", op);
            cpu->PC += 2;
            if (cpu->V[OP_X(op)] != cpu->V[OP_Y(op)])
                skip_instruction(cpu);
            break;
        case 0xA000: /* ANNN - LD I, addr */
            TRACE_CPU(conf, cpu, "LD_I", op);
            cpu->I = OP_NNN(op);
            cpu->PC += 2;
            break;
        case 0xB000: /* BNNN - JP V0, addr */
            // Schip and xochip change this instruction behavior
            // to use vx offset instead of v0
            if (conf->jump_quirk) {
                uint16_t xnn = op & 0x0FFF;
                uint8_t x = OP_X(op);
                TRACE_CPU(conf, cpu, "JP_VX", op);
                cpu->PC = xnn + cpu->V[x];
            } else {
                TRACE_CPU(conf, cpu, "JP_V0", op);
                cpu->PC = OP_NNN(op) + cpu->V[0];
            }
            break;
        case 0xC000: /* CXKK - RND Vx, byte */
            TRACE_CPU(conf, cpu, "RND", op);
            cpu->V[OP_X(op)] = (rand() % 256) & OP_KK(op);
            cpu->PC += 2;
            break;
        case 0xD000: { /* DXYN - DRW Vx, Vy, nibble */
            TRACE_CPU(conf, cpu, "DRW", op);

            int width = cpu->hires_mode ? 128 : 64;
            int height = cpu->hires_mode ? 64 : 32;

            // Wrap starting coordinates right away as required by standard specs
            uint8_t x = cpu->V[OP_X(op)] % width;
            uint8_t y = cpu->V[OP_Y(op)] % height;
            uint8_t n = OP_N(op);

            cpu->V[0xF] = 0;

            // Zero size means 16x16 mega sprite mode in schip and xochip specs
            int sprite_height = (n == 0) ? 16 : n;
            int sprite_width = (n == 0) ? 16 : 8;
            int bytes_per_row = (n == 0) ? 2 : 1;

            uint16_t current_I = cpu->I;

            for (int plane = 0; plane < 2; plane++) {
                if (!(cpu->selected_plane & (1 << plane)))
                    continue;

                for (int row = 0; row < sprite_height; row++) {
                    uint16_t sprite_row;

                    if (n == 0)
                        sprite_row = (cpu->memory[current_I + (row * 2)] << 8) |
                                                        cpu->memory[current_I + (row * 2) + 1];
                    else
                        sprite_row = cpu->memory[current_I + row] << 8;

                    for (int col = 0; col < sprite_width; col++) {
                        // Check individual pixel bits using sliding bitmask
                        if ((sprite_row & (0x8000 >> col)) != 0) {
                            int px = x + col;
                            int py = y + row;

                            // Old chip8 wraps pixels dragging out of bounds while new specs clip them
                            if (!conf->clip_quirk) {
                                px %= width;
                                py %= height;
                            } else {
                                if (px >= width || py >= height)
                                    continue;
                            }

                            int vid_index = (plane * 8192) + px + (py * width);
                            if (cpu->VIDEO[vid_index] == 1)
                                cpu->V[0xF] = 1;
                            cpu->VIDEO[vid_index] ^= 1;
                        }
                    }
                }

                current_I += sprite_height * bytes_per_row;
            }

            // Index register auto increments only when both
            // planes are active simultaneously
            if (cpu->selected_plane == 3)
                cpu->I = current_I;

            cpu->draw_flag = 1;
            if (conf->disp_wait)
                cpu->disp_wait = 0;
            cpu->PC += 2;
            break;
        }
        case 0xE000:
            switch (OP_KK(op)) {
                case 0x009E: /* EX9E - SKP Vx */
                    TRACE_CPU(conf, cpu, "SKP", op);
                    cpu->PC += 2;
                    if (cpu->KEYPAD[cpu->V[OP_X(op)]] != 0)
                        skip_instruction(cpu);
                    break;
                case 0x00A1: /* EXA1 - SKNP Vx */
                    TRACE_CPU(conf, cpu, "SKNP", op);
                    cpu->PC += 2;
                    if (cpu->KEYPAD[cpu->V[OP_X(op)]] == 0)
                        skip_instruction(cpu);
                    break;
                default:
                    UNKNOWN_OPCODE(op);
            }
            break;
        case 0xF000:
            if (OP_KK(op) == 0x0000) { /* F000 NNNN - I = NNNN */
                TRACE_CPU(conf, cpu, "FN_LONG", op);
                cpu->I = (cpu->memory[cpu->PC + 2] << 8) | cpu->memory[cpu->PC + 3];
                cpu->PC += 4;
                break;
            }
            switch (OP_KK(op)) {
                case 0x0001: /* FN01 - PLANE n */
                    TRACE_CPU(conf, cpu, "PLANE", op);
                    cpu->selected_plane = OP_X(op);
                    cpu->PC += 2;
                    break;
                case 0x0002: /* F002 - AUDIO */
                    TRACE_CPU(conf, cpu, "AUDIO", op);
                    memcpy(cpu->audio_pattern, &cpu->memory[cpu->I], 16);
                    cpu->PC += 2;
                    break;
                case 0x0007: /* FX07 - LD Vx, DT */
                    TRACE_CPU(conf, cpu, "LD_VX_DT", op);
                    cpu->V[OP_X(op)] = cpu->DT;
                    cpu->PC += 2;
                    break;
                case 0x000A: { /* FX0A - LD Vx, K */
                    TRACE_CPU(conf, cpu, "LD_VX_K", op);
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
                    TRACE_CPU(conf, cpu, "LD_DT_VX", op);
                    cpu->DT = cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x0018: /* FX18 - LD ST, Vx */
                    TRACE_CPU(conf, cpu, "LD_ST_VX", op);
                    cpu->ST = cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x001E: /* FX1E - ADD I, Vx */
                    TRACE_CPU(conf, cpu, "ADD_I_VX", op);
                    cpu->I += cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x0029: /* FX29 - LD F, Vx */
                    TRACE_CPU(conf, cpu, "LD_F_VX", op);
                    cpu->I = cpu->V[OP_X(op)] * 5;
                    cpu->PC += 2;
                    break;
                case 0x0030: /* FX30 or FE30 - LD HF, Vx */
                    TRACE_CPU(conf, cpu, "LD_HF_VX", op);
                    // Schip high res characters start after normal fonts
                    // and are 10 bytes tall
                    cpu->I = 80 + (cpu->V[OP_X(op)] * 10); 
                    cpu->PC += 2;
                    break;
                case 0x0033: { /* FX33 - LD B, Vx */
                    TRACE_CPU(conf, cpu, "LD_B_VX", op);
                    uint8_t value = cpu->V[OP_X(op)];
                    cpu->memory[cpu->I] = value / 100;
                    cpu->memory[cpu->I + 1] = (value / 10) % 10;
                    cpu->memory[cpu->I + 2] = value % 10;
                    cpu->PC += 2;
                    break;
                }
                case 0x003A: /* FX3A - PITCH Vx */
                    TRACE_CPU(conf, cpu, "PITCH", op);
                    cpu->audio_pitch = cpu->V[OP_X(op)];
                    cpu->PC += 2;
                    break;
                case 0x0055: { /* FX55 - LD [I], Vx */
                    TRACE_CPU(conf, cpu, "LD_MEM_VX", op);
                    int x = OP_X(op);
                    for (int i = 0; i <= x; i++)
                        cpu->memory[cpu->I + i] = cpu->V[i];

                    if (conf->memory_quirk)
                        cpu->I += x + 1;

                    cpu->PC += 2;
                    break;
                }
                case 0x0065: { /* FX65 - LD Vx, [I] */
                    TRACE_CPU(conf, cpu, "LD_VX_MEM", op);
                    int x = OP_X(op);
                    for (int i = 0; i <= x; i++)
                        cpu->V[i] = cpu->memory[cpu->I + i];

                    if (conf->memory_quirk)
                        cpu->I += x + 1;

                    cpu->PC += 2;
                    break;
                }
                case 0x0075: { /* FX75 - Save V0-VX to RPL */
                    TRACE_CPU(conf, cpu, "SCHIP_SAVE_RPL", op);
                    int x = OP_X(op);
                    for (int i = 0; i <= x; i++)
                        cpu->RPL[i] = cpu->V[i];
                    cpu->PC += 2;
                    break;
                }
                case 0x0085: { /* FX85 - Load V0-VX from RPL */
                    TRACE_CPU(conf, cpu, "SCHIP_LOAD_RPL", op);
                    int x = OP_X(op);
                    for (int i = 0; i <= x; i++)
                        cpu->V[i] = cpu->RPL[i];
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
