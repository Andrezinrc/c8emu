#include "cpu.h"
#include "video.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Use: ./mychip8 path/to/rom.ch8\n");
        return 1;
    }

    struct Chip8 cpu;
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;

    cpu_init(&cpu);
    vid_init(&win, &ren);

    char *ROM = argv[1];
    if(!cpu_load_rom(&cpu, ROM)) {
        printf("ROM not found!\n");
        vid_close(win, ren);
        return 1;
    }

    // audio
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S8;
    want.channels = 1;
    want.samples = 512;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    SDL_PauseAudioDevice(dev, 1);

    int8_t som_buffer[44100];
    for (int i = 0; i < 44100; i++) {
        som_buffer[i] = ((i / 50) % 2 == 0) ? 50 : -50;
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
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                uint8_t state = (event.type == SDL_KEYDOWN) ? 1 : 0;

                // keypad
                switch (event.key.keysym.sym)
                {
                    case SDLK_1:
                    {
                        cpu.KEYPAD[0x1] = state;
                        break;
                    }
                    case SDLK_2:
                    {
                        cpu.KEYPAD[0x2] = state;
                        break;
                    }
                    case SDLK_3:
                    {
                        cpu.KEYPAD[0x3] = state;
                        break;
                    }
                    case SDLK_4:
                    {
                        cpu.KEYPAD[0xC] = state;
                        break;
                    }
                    case SDLK_q:
                    {
                        cpu.KEYPAD[0x4] = state;
                        break;
                    }
                    case SDLK_w:
                    {
                        cpu.KEYPAD[0x5] = state;
                        break;
                    }
                    case SDLK_e:
                    {
                        cpu.KEYPAD[0x6] = state;
                        break;
                    }
                    case SDLK_r:
                    {
                        cpu.KEYPAD[0xD] = state;
                        break;
                    }
                    case SDLK_a:
                    {
                        cpu.KEYPAD[0x7] = state;
                        break;
                    }
                    case SDLK_s:
                    {
                        cpu.KEYPAD[0x8] = state;
                        break;
                    }
                    case SDLK_d:
                    {
                        cpu.KEYPAD[0x9] = state;
                        break;
                    }
                    case SDLK_f:
                    {
                        cpu.KEYPAD[0xE] = state;
                        break;
                    }
                    case SDLK_z:
                    {
                        cpu.KEYPAD[0xA] = state;
                        break;
                    }
                    case SDLK_x:
                    {
                        cpu.KEYPAD[0x0] = state;
                        break;
                    }
                    case SDLK_c:
                    {
                        cpu.KEYPAD[0xB] = state;
                        break;
                    }
                    case SDLK_v:
                    {
                        cpu.KEYPAD[0xF] = state;
                        break;
                    }
                }
            } 
        }

        uint32_t current_time = SDL_GetTicks();
        uint32_t elapsed_time = current_time - last_frame_time;
        last_frame_time = current_time;
        
        //cpu_acc += elapsed_time * 0.3f; // 300Hz
        //timer_acc += elapsed_time;

        cpu_acc += elapsed_time * 3;
        timer_acc += elapsed_time;

        //total_cls += cpu_cycles(&cpu, ren, &cpu_acc);

        // ~60Hz
        while (timer_acc >= 16)
        {
            cpu_update_timers(&cpu, dev, som_buffer);
            timer_acc -= 16;
            frames++;
        }

        int cycles_to_run = (int)cpu_acc / 10;
        cpu_acc = (int)cpu_acc % 10;

        for (int i = 0; i < cycles_to_run; i++) {
            cpu_step(&cpu);
            total_cls++;
        }

        if (cpu.draw_flag) {
            vid_update(ren, &cpu);
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
    SDL_CloseAudioDevice(dev);

    return 0;

}
