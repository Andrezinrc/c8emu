#include "video.h"

SDL_Texture *texture = NULL;

int vid_init(SDL_Window **win, SDL_Renderer **ren, struct Config *conf)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;

    *win = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                            64 * conf->window_scale, 32 * conf->window_scale, 0);

    if (!*win) return 0;

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED);
    if (!*ren) return 0;

    texture = SDL_CreateTexture(*ren, SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (!texture) return 0;

    return 1;
}

void vid_update(SDL_Renderer *ren, struct Chip8 *cpu, struct Config *conf)
{
    uint32_t pixel_buffer[64 * 32];

    for (int i = 0; i < 2048; i++)
        if (cpu->VIDEO[i])
            pixel_buffer[i] = conf->fg_color;
        else
            pixel_buffer[i] = conf->bg_color;

    SDL_RenderClear(ren);
    SDL_UpdateTexture(texture, NULL, pixel_buffer, 64 * sizeof(uint32_t));
    SDL_RenderCopy(ren, texture, NULL, NULL);
    SDL_RenderPresent(ren);
}

void vid_close(SDL_Window *win, SDL_Renderer *ren)
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (ren)
        SDL_DestroyRenderer(ren);
    if (win)
        SDL_DestroyWindow(win);
    SDL_Quit();
}
