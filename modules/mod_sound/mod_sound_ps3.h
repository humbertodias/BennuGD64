#ifndef __MOD_SOUND_PS3_H
#define __MOD_SOUND_PS3_H

#include <SDL3/SDL.h>
#include "files.h"

void           modsound_ps3_prepare( void );
void           modsound_ps3_adjust_spec( SDL_AudioSpec * spec );
SDL_IOStream * modsound_ps3_slurp_file( file * fp );

#endif
