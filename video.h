#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL.h>
#include "cpu.h"

#define SCALE 5

#define PIXEL_ON_COLOR 0xFFFFFFFF
#define PIXEL_OFF_COLOR 0x00000000

struct Chip8;

int vid_init(SDL_Window **win, SDL_Renderer **ren);
void vid_update(SDL_Renderer *ren, struct Chip8 *cpu);
void vid_close(SDL_Window *win, SDL_Renderer *ren);

#endif
