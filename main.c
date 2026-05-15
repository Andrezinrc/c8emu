#include "cpu.h"
#include "display.h"
#include <stdio.h>

int main() {
    struct Chip8 cpu;
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;

    cpu_init(&cpu);

    if (!display_init(&win, &ren) || !cpu_load_rom(&cpu, "Chip8 emulator Logo [Garstyciuks].ch8")) {
        return 1;
    }

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        cpu_cycles(&cpu, ren, 9);
        cpu_update_timers(&cpu);

        SDL_Delay(16);
    }

    return 0;

}
