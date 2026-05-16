#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include "cpu.h"

#define SCALE 15

struct Chip8;

int display_init(SDL_Window **win, SDL_Renderer **ren);
void display_update(SDL_Renderer *ren, struct Chip8 *cpu);
void display_close(SDL_Window *win, SDL_Renderer *ren);

#endif
