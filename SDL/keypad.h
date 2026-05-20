#ifndef KEYPAD_H
#define KEYPAD_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "../cpu.h"

void key_update(struct Chip8 *cpu, SDL_Event *event);

#endif
