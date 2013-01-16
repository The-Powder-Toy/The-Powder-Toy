#ifndef SOUND_H
#define SOUND_H
#include "Config.h"
#include "SDL_audio.h"
void add_note(float f);
void create_tone(void *userdata, Uint8 *stream, int l);
#endif