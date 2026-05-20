#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>
#include <stdint.h>

SDL_AudioDeviceID aud_init(int8_t *som_buffer);
void aud_close(SDL_AudioDeviceID aud_dev);

#endif
