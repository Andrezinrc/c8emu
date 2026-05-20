#include "keypad.h"

void key_update(struct Chip8 *cpu, SDL_Event *event) {
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        uint8_t state = (event->type == SDL_KEYDOWN) ? 1 : 0;

        switch (event->key.keysym.sym)
        {
            case SDLK_1:
            {
                cpu->KEYPAD[0x1] = state;
                break;
            }
            case SDLK_2:
            {
                cpu->KEYPAD[0x2] = state;
                break;
            }
            case SDLK_3:
            {
                cpu->KEYPAD[0x3] = state;
                break;
            }
            case SDLK_4:
            {
                cpu->KEYPAD[0xC] = state;
                break;
            }
            case SDLK_q:
            {
                cpu->KEYPAD[0x4] = state;
                break;
            }
            case SDLK_w:
            {
                cpu->KEYPAD[0x5] = state;
                break;
            }
            case SDLK_e:
            {
                cpu->KEYPAD[0x6] = state;
                break;
            }
            case SDLK_r:
            {
                cpu->KEYPAD[0xD] = state;
                break;
            }
            case SDLK_a:
            {
                cpu->KEYPAD[0x7] = state;
                break;
            }
            case SDLK_s:
            {   
                cpu->KEYPAD[0x8] = state;
                break;
            }
            case SDLK_d:
            {
                cpu->KEYPAD[0x9] = state;
                break;
            }
            case SDLK_f:
            {
                cpu->KEYPAD[0xE] = state;
                break;
            }
            case SDLK_z:
            {
                cpu->KEYPAD[0xA] = state;
                break;
            }
            case SDLK_x:
            {
                cpu->KEYPAD[0x0] = state;
                break;
            }
            case SDLK_c:
            {
                cpu->KEYPAD[0xB] = state;
                break;
            }
            case SDLK_v:
            {
                cpu->KEYPAD[0xF] = state;
                break;
            }
        }
    }
}
