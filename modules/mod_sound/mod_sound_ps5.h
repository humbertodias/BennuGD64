#ifndef __MOD_SOUND_PS5_H
#define __MOD_SOUND_PS5_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "files.h"

void           modsound_ps5_adjust_spec( SDL_AudioSpec * spec );
#ifndef TARGET_LIBRETRO
int            modsound_ps5_start_output( MIX_Mixer * mixer );
void           modsound_ps5_stop_output( void );
#endif
SDL_IOStream * modsound_ps5_slurp_file( file * fp );

#endif
