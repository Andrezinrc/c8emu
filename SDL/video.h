#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL.h>
#include "../cpu.h"
#include "../config.h"

struct Chip8;

int vid_init(SDL_Window **win, SDL_Renderer **ren, struct Config *conf);
void vid_update(SDL_Renderer *ren, struct Chip8 *cpu);
void vid_close(SDL_Window *win, SDL_Renderer *ren);

#endif
