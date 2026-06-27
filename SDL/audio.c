#include "audio.h"

SDL_AudioDeviceID aud_init(int8_t *som_buffer)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) return 0;

    SDL_AudioSpec want;
    SDL_zero(want);
    
    want.freq = 44100;
    want.format = AUDIO_S8;
    want.channels = 1;
    want.samples = 512;

    SDL_AudioDeviceID aud_dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    
    for (int i = 0; i < 44100; i++)
        som_buffer[i] = ((i / 50) % 2 == 0) ? 50 : -50;

    SDL_QueueAudio(aud_dev, som_buffer, 44100);

    return aud_dev;
}

void aud_close(SDL_AudioDeviceID aud_dev)
{
    if (aud_dev != 0)
        SDL_CloseAudioDevice(aud_dev);
}
