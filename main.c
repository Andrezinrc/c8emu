#include "cpu.h"
#include "config.h"
#include "SDL/video.h"
#include "SDL/audio.h"
#include "SDL/keypad.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("./c8emu path/to/rom.ch8\n");
        return 1;
    }

    struct Chip8 cpu;
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    
    cpu_init(&cpu);

    struct Config conf;
    conf.cpu_hz =       10;
    conf.vf_reset  =    1;
    conf.memory_quirk = 1;
    conf.disp_wait =    1;
    conf.clip_quirk   = 1;
    conf.shift_quirk =  0;
    conf.window_scale = 10;
    conf.trace =        0;
    conf.fg_color =     0xFFFFFFFF;
    conf.bg_color =     0x00000000;
    print_config(&conf);

    vid_init(&win, &ren, &conf);

    int8_t som_buffer[44100];
    SDL_AudioDeviceID aud_dev = aud_init(som_buffer);

    char *ROM = argv[1];
    if(!cpu_load_rom(&cpu, ROM)) {
        printf("ROM not found!\n");
        vid_close(win, ren);
        aud_close(aud_dev);
        return 1;
    }

    uint32_t last_frame_time = SDL_GetTicks();
    uint32_t last_fps_time = SDL_GetTicks();

    int frames = 0;
    int total_cls = 0;
    int running = 1;

    int cpu_acc = 0;
    int timer_acc = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            key_update(&cpu, &event);
        }

        uint32_t current_time = SDL_GetTicks();
        uint32_t elapsed_time = current_time - last_frame_time;
        last_frame_time = current_time;
        
        //cpu_acc += elapsed_time * 0.3f; // 300Hz
        //timer_acc += elapsed_time;

        cpu_acc += elapsed_time * conf.cpu_hz;
        timer_acc += elapsed_time;

        //total_cls += cpu_cycles(&cpu, ren, &cpu_acc);

        // ~60Hz
        while (timer_acc >= 16)
        {
            cpu_update_timers(&cpu, aud_dev, som_buffer);
            timer_acc -= 16;
            frames++;
            cpu.disp_wait = 1;
        }

        int cycles_to_run = (int)cpu_acc / 10;
        cpu_acc = (int)cpu_acc % 10;

        for (int i = 0; i < cycles_to_run; i++) {
            cpu_step(&cpu, &conf);
            total_cls++;
        }
        if (cpu.draw_flag) {
            vid_update(ren, &cpu, &conf);
            cpu.draw_flag = 0;
        }
        if (current_time - last_fps_time >= 1000)
        {
            //printf("VIDEO: %d Hz | CPU: %d Hz\n", frames, total_cls);
            
            frames = 0;
            total_cls = 0;
            last_fps_time = current_time;
        }

        SDL_Delay(1);
    }

    vid_close(win, ren);
    aud_close(aud_dev);

    return 0;

}
