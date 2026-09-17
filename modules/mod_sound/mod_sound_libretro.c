#include <string.h>
#include "mod_sound_libretro.h"

static MIX_Mixer * libretro_mixer;
static int libretro_rate = 44100;

int libretro_audio_sample_rate( void )
{
    return libretro_rate;
}

int modsound_libretro_start_output( MIX_Mixer * mixer )
{
    SDL_AudioSpec spec;
    libretro_mixer = mixer;
    if ( mixer && MIX_GetMixerFormat( mixer, &spec ) && spec.freq > 0 )
        libretro_rate = spec.freq;
    return 0;
}

void libretro_audio_mix( void * mixbuf, size_t mixbuf_size )
{
    if ( !mixbuf || !mixbuf_size )
        return;
    if ( !libretro_mixer )
    {
        memset( mixbuf, 0, mixbuf_size );
        return;
    }
    if ( MIX_Generate( libretro_mixer, mixbuf, ( int ) mixbuf_size ) < 0 )
        memset( mixbuf, 0, mixbuf_size );
}
