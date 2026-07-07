#include "video.h"

SDL_Texture *texture = NULL;
int current_width = 0;
int current_height = 0;

int vid_init(SDL_Window **win, SDL_Renderer **ren, struct Config *conf)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;

    *win = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                            64 * conf->window_scale, 32 * conf->window_scale, 0);

    if (!*win) return 0;

    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED);
    if (!*ren) return 0;

    // Nearest neighbor filtering prevents pixels from blurring out when upscaled
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    texture = SDL_CreateTexture(*ren, SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (!texture) return 0;

    current_width = 64;
    current_height = 32;

    return 1;
}

void vid_update(SDL_Renderer *ren, struct Chip8 *cpu)
{
    int width = cpu->hires_mode ? 128 : 64;
    int height = cpu->hires_mode ? 64 : 32;

    // Recreate target buffer texture whenever resolution
    // toggles between lores and hires modes
    if (width != current_width || height != current_height) {
        if (texture) SDL_DestroyTexture(texture);
        texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_STREAMING, width, height);
        current_width = width;
        current_height = height;
    }

    uint32_t pixel_buffer[width * height];

    uint32_t colors[4] = {
        0x100010FF,
        0xFFFFFFFF,
        0xFF1493FF,
        0xFFFFFFFF
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int p1_val = cpu->VIDEO[x + (y * width)];
            // Second color plane data offset is anchored at 8192 byte boundary marks
            int p2_val = cpu->VIDEO[8192 + x + (y * width)];
            // Combine plane bits to generate a 2 bit index mapping onto the four xochip colors
            int color_idx = p1_val | (p2_val << 1);
            pixel_buffer[x + (y * width)] = colors[color_idx];
        }
    }

    SDL_RenderClear(ren);
    SDL_UpdateTexture(texture, NULL, pixel_buffer, width * sizeof(uint32_t));
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
