#include "cpu.h"
#include "config.h"
#include "SDL/video.h"
#include "SDL/audio.h"
#include "SDL/keypad.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("./chip8 path/to/rom.ch8 [-chip8 | -schip | -xochip] [-scale 5-20]\n");
        return 1;
    }

    struct Chip8 cpu;
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    
    cpu_init(&cpu);

    struct Config conf;
    conf.cpu_trace = 0;

    int scale = 5;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-schip") == 0)
            conf.mode = MODE_SCHIP;
        else if (strcmp(argv[i], "-xochip") == 0)
            conf.mode = MODE_XOCHIP;
        else if (strcmp(argv[i], "-chip8") == 0)
            conf.mode = MODE_CHIP8;
        else if (strcmp(argv[i], "-scale") == 0) {
            if (i + 1 < argc) {
                int temp_scale = atoi(argv[i + 1]);
                if (temp_scale < 5) {
                    printf("Scale %d is too small. Setting to 5\n", temp_scale);
                    scale = 5;
                } else if (temp_scale > 20) {
                    printf("Scale %d is too large. Setting to 20\n", temp_scale);
                    scale = 20;
                } else {
                    scale = temp_scale;
                }
                i++;
            }
        }
    }

    conf.window_scale = scale;
    apply_mode_config(&conf);
    vid_init(&win, &ren, &conf);

    int8_t som_buffer[44100];
    SDL_AudioDeviceID aud_dev = aud_init(som_buffer);

    char *ROM = argv[1];
    
    FILE *f_check = fopen(ROM, "rb");
    long rom_size = 0;
    if (f_check) {
        fseek(f_check, 0, SEEK_END);
        rom_size = ftell(f_check);
        fclose(f_check);
    }

    if(!cpu_load_rom(&cpu, ROM)) {
        printf("ROM not found!\n");
        vid_close(win, ren);
        aud_close(aud_dev);
        return 1;
    }

    print_config(&conf, ROM, rom_size);

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
            vid_update(ren, &cpu);
            cpu.draw_flag = 0;
        }
        if (current_time - last_fps_time >= 1000)
        {
            //printf("VIDEO: %d fps | CPU: %d Hz\n", frames, total_cls);
            
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
