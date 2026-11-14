/*
 * Zeebo has no SDL playback device. Mix into a buffer and hand the PCM to
 * the emulator in one call, the same way the frame is presented.
 */

#include <stdint.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "mod_sound_zeebo.h"

#define ZEEBO_MODULE_BASE 0x00010000u
#define ZEEBO_AUDIO_SLOT 118
/* One slice, stereo S16 at 48 kHz. Longer gaps are split so the queue stays short. */
#define ZEEBO_AUDIO_BYTES (48000 / 25 * 2 * 2)

static MIX_Mixer * zeebo_mixer;
static Uint32 zeebo_last_ms;
static int zeebo_have_last;
static uint8_t zeebo_pcm[ZEEBO_AUDIO_BYTES];

typedef uint32_t ( *zeebo_audio_fn )( const void * pcm, uint32_t bytes, uint32_t rate, uint32_t channels );

static int zeebo_push( const void * pcm, uint32_t bytes, uint32_t rate, int channels )
{
    uint32_t ver = *( volatile uint32_t * )( uintptr_t )( ZEEBO_MODULE_BASE - 8 );
    void ** table;
    zeebo_audio_fn fn;

    if ( ver < 3 || !pcm || bytes == 0 || rate == 0 || channels <= 0 )
        return 0;
    table = *( void *** )( uintptr_t )( ZEEBO_MODULE_BASE - 4 );
    if ( !table )
        return -1;
    fn = ( zeebo_audio_fn )( uintptr_t )table[ZEEBO_AUDIO_SLOT];
    if ( !fn )
        return -1;
    return fn( pcm, bytes, rate, ( uint32_t )channels ) == 0 ? 0 : -1;
}

int modsound_zeebo_start_output( MIX_Mixer * mixer )
{
    if ( !mixer )
        return -1;
    zeebo_mixer = mixer;
    zeebo_have_last = 0;
    return 0;
}

void modsound_zeebo_stop_output( void )
{
    zeebo_mixer = NULL;
    zeebo_have_last = 0;
}

void modsound_zeebo_pump( void )
{
    SDL_AudioSpec spec;
    Uint32 now, elapsed;
    int channels, frames, bytes;

    if ( !zeebo_mixer )
        return;
    if ( !MIX_GetMixerFormat( zeebo_mixer, &spec ) || spec.freq <= 0 || spec.channels <= 0 )
        return;

    now = SDL_GetTicks();
    if ( !zeebo_have_last )
    {
        zeebo_last_ms = now;
        zeebo_have_last = 1;
        return;
    }
    elapsed = now - zeebo_last_ms;
    if ( elapsed < 5 )
        return;
    if ( elapsed > 40 )
        elapsed = 40;
    zeebo_last_ms = now;

    channels = spec.channels;
    frames = ( int )( ( uint64_t )spec.freq * elapsed / 1000 );
    if ( frames < 1 )
        return;
    bytes = frames * channels * 2;
    if ( bytes > ZEEBO_AUDIO_BYTES )
    {
        frames = ZEEBO_AUDIO_BYTES / ( channels * 2 );
        bytes = frames * channels * 2;
    }
    if ( MIX_Generate( zeebo_mixer, zeebo_pcm, bytes ) < 0 )
        return;
    zeebo_push( zeebo_pcm, ( uint32_t )bytes, ( uint32_t )spec.freq, channels );
}
