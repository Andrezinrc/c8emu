#include "cpu.h"
#include "display.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Use: ./mychip8 /path/to/rom.ch8");
        return 1;
    }

    struct Chip8 cpu;
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;

    cpu_init(&cpu);
    display_init(&win, &ren);

    char *ROM = argv[1];
    if(!cpu_load_rom(&cpu, ROM)) {
        printf("ROM not found!\n");
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

    display_close(win, ren);

    return 0;

}
