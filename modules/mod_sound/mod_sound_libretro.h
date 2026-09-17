#ifndef __MOD_SOUND_LIBRETRO_H
#define __MOD_SOUND_LIBRETRO_H

#include <SDL3_mixer/SDL_mixer.h>

int  libretro_audio_sample_rate( void );
void libretro_audio_mix( void * mixbuf, size_t mixbuf_size );
int  modsound_libretro_start_output( MIX_Mixer * mixer );

#endif
