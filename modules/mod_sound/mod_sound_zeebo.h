#ifndef __MOD_SOUND_ZEEBO_H
#define __MOD_SOUND_ZEEBO_H

#include <SDL3_mixer/SDL_mixer.h>

int  modsound_zeebo_start_output( MIX_Mixer * mixer );
void modsound_zeebo_stop_output( void );
void modsound_zeebo_pump( void );

#endif
