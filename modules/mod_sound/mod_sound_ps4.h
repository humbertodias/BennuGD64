#ifndef __MOD_SOUND_PS4_H
#define __MOD_SOUND_PS4_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "files.h"

void           modsound_ps4_prepare( void );
void           modsound_ps4_adjust_spec( SDL_AudioSpec * spec );
int            modsound_ps4_start_output( MIX_Mixer * mixer );
void           modsound_ps4_stop_output( void );
SDL_IOStream * modsound_ps4_slurp_file( file * fp );

#endif
