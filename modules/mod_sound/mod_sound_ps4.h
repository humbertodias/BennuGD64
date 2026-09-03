#ifndef __MOD_SOUND_PS4_H
#define __MOD_SOUND_PS4_H

#include <SDL3/SDL.h>
#include "files.h"

void           modsound_ps4_prepare( void );
void           modsound_ps4_adjust_spec( SDL_AudioSpec * spec );
SDL_IOStream * modsound_ps4_slurp_file( file * fp );

#endif
