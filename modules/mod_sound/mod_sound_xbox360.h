#ifndef __MOD_SOUND_XBOX360_H
#define __MOD_SOUND_XBOX360_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

void modsound_xbox360_prepare( void );
void modsound_xbox360_adjust_spec( SDL_AudioSpec * spec );
int  modsound_xbox360_start_output( MIX_Mixer * mixer );
void modsound_xbox360_stop_output( void );
void modsound_xbox360_pump( void );

#endif
