/*
 * Xbox 360 native audio sink. libXenon has no pthread runtime, so the module
 * fills its non-blocking hardware queue from the interpreter frame hook.
 */

#include <stdint.h>
#include <string.h>

#include <xenon_sound/sound.h>

#include "mod_sound_xbox360.h"

enum
{
    XBOX360_AUDIO_RATE = 48000,
    XBOX360_AUDIO_CHANNELS = 2,
    XBOX360_AUDIO_FRAMES = 1024,
    XBOX360_AUDIO_BYTES = XBOX360_AUDIO_FRAMES * XBOX360_AUDIO_CHANNELS * 2
};

static MIX_Mixer * xbox360_mixer;
static int16_t xbox360_samples[XBOX360_AUDIO_FRAMES * XBOX360_AUDIO_CHANNELS]
    __attribute__((aligned(128)));

void modsound_xbox360_prepare( void )
{
    xenon_sound_init();
}

void modsound_xbox360_adjust_spec( SDL_AudioSpec * spec )
{
    if ( !spec ) return;
    spec->freq = XBOX360_AUDIO_RATE;
    spec->format = SDL_AUDIO_S16LE;
    spec->channels = XBOX360_AUDIO_CHANNELS;
}

int modsound_xbox360_start_output( MIX_Mixer * mixer )
{
    if ( !mixer ) return -1;
    xbox360_mixer = mixer;
    memset( xbox360_samples, 0, sizeof( xbox360_samples ) );
    modsound_xbox360_pump();
    return 0;
}

void modsound_xbox360_stop_output( void )
{
    xbox360_mixer = NULL;
}

void modsound_xbox360_pump( void )
{
    int submitted = 0;
    if ( !xbox360_mixer ) return;

    while ( xenon_sound_get_free() >= XBOX360_AUDIO_BYTES && submitted < 4 )
    {
        if ( MIX_Generate( xbox360_mixer, xbox360_samples,
                           XBOX360_AUDIO_BYTES ) < 0 )
            memset( xbox360_samples, 0, sizeof( xbox360_samples ) );
        xenon_sound_submit( xbox360_samples, XBOX360_AUDIO_BYTES );
        submitted++;
    }
}
