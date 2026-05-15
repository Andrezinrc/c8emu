#include "display.h"

int display_init(SDL_Window **win, SDL_Renderer **ren) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;

    *win = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64 * SCALE, 32 * SCALE, 0);

    if (!*win) return 0;

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED);
    if (!*ren) return 0;

    return 1;
}

void display_update(SDL_Renderer *ren, struct Chip8 *cpu) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (cpu->display[x][y]) {
                SDL_Rect rect;
                rect.x = x * SCALE;
                rect.y = y * SCALE;
                rect.w = SCALE;
                rect.h = SCALE;
                SDL_RenderFillRect(ren, &rect);
            }
        }
    }
    SDL_RenderPresent(ren);
}

void display_close(SDL_Window *win, SDL_Renderer *ren) {
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
}
