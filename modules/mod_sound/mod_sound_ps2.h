#ifndef __MOD_SOUND_PS2_H
#define __MOD_SOUND_PS2_H

#include <SDL3/SDL.h>
#include "files.h"

int            modsound_ps2_skip_audio( void );
int            modsound_ps2_skip_song( void );
void           modsound_ps2_prepare( void );
void           modsound_ps2_adjust_rate( int * audio_rate );
SDL_IOStream * modsound_ps2_slurp_file( file * fp );

#endif
