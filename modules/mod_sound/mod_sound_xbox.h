#ifndef __MOD_SOUND_XBOX_H
#define __MOD_SOUND_XBOX_H

#include <SDL3_mixer/SDL_mixer.h>

void modsound_xbox_prepare( void );
void modsound_xbox_adjust_spec( SDL_AudioSpec * spec );
int  modsound_xbox_start_output( MIX_Mixer * mixer );
void modsound_xbox_stop_output( void );
void modsound_xbox_pump( void );

#endif
