/*
 * Original Xbox native audio sink via nxdk XAudio.
 * The AC97 callback only copies from a prefilled ring; MIX_Generate runs on
 * the interpreter frame hook to keep FPU work off the DPC path.
 */

#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <hal/audio.h>
#include <xboxkrnl/xboxkrnl.h>

#include "mod_sound_xbox.h"

enum
{
    XBOX_AUDIO_RATE = 48000,
    XBOX_AUDIO_CHANNELS = 2,
    XBOX_AUDIO_FRAMES = 1024,
    XBOX_AUDIO_BYTES = XBOX_AUDIO_FRAMES * XBOX_AUDIO_CHANNELS * 2,
    XBOX_AUDIO_BUFFERS = 4
};

static MIX_Mixer * xbox_mixer;
static uint8_t * xbox_buffers[XBOX_AUDIO_BUFFERS];
static volatile int xbox_write;
static volatile int xbox_read;
static volatile int xbox_filled;
static int xbox_audio_ready;

#define MAXRAM 0x03FFAFFF

static void xbox_audio_callback( void * pac97Device, void * data )
{
    uint8_t * src;
    ( void )pac97Device;
    ( void )data;

    if ( xbox_filled <= 0 )
    {
        memset( xbox_buffers[0], 0, XBOX_AUDIO_BYTES );
        XAudioProvideSamples( xbox_buffers[0], ( unsigned short )XBOX_AUDIO_BYTES, 0 );
        return;
    }

    src = xbox_buffers[xbox_read];
    XAudioProvideSamples( src, ( unsigned short )XBOX_AUDIO_BYTES, 0 );
    xbox_read = ( xbox_read + 1 ) % XBOX_AUDIO_BUFFERS;
    xbox_filled--;
}

void modsound_xbox_prepare( void )
{
    int i;

    if ( xbox_audio_ready ) return;

    for ( i = 0; i < XBOX_AUDIO_BUFFERS; i++ )
    {
        xbox_buffers[i] = ( uint8_t * )MmAllocateContiguousMemoryEx(
            XBOX_AUDIO_BYTES, 0, MAXRAM, 0,
            ( PAGE_READWRITE | PAGE_WRITECOMBINE ) );
        if ( xbox_buffers[i] )
            memset( xbox_buffers[i], 0, XBOX_AUDIO_BYTES );
    }

    xbox_write = 0;
    xbox_read = 0;
    xbox_filled = 0;
    XAudioInit( 16, 2, xbox_audio_callback, NULL );
    xbox_audio_ready = 1;
}

void modsound_xbox_adjust_spec( SDL_AudioSpec * spec )
{
    if ( !spec ) return;
    spec->freq = XBOX_AUDIO_RATE;
    spec->format = SDL_AUDIO_S16LE;
    spec->channels = XBOX_AUDIO_CHANNELS;
}

int modsound_xbox_start_output( MIX_Mixer * mixer )
{
    int i;
    if ( !mixer ) return -1;
    for ( i = 0; i < XBOX_AUDIO_BUFFERS; i++ )
        if ( !xbox_buffers[i] ) return -1;

    xbox_mixer = mixer;
    xbox_write = 0;
    xbox_read = 0;
    xbox_filled = 0;
    modsound_xbox_pump();
    for ( i = 0; i < 2 && xbox_filled > 0; i++ )
        xbox_audio_callback( NULL, NULL );
    XAudioPlay();
    return 0;
}

void modsound_xbox_stop_output( void )
{
    XAudioPause();
    xbox_mixer = NULL;
    xbox_filled = 0;
}

void modsound_xbox_pump( void )
{
    if ( !xbox_mixer || !xbox_audio_ready ) return;

    while ( xbox_filled < XBOX_AUDIO_BUFFERS - 1 )
    {
        uint8_t * dst = xbox_buffers[xbox_write];
        if ( MIX_Generate( xbox_mixer, dst, XBOX_AUDIO_BYTES ) < 0 )
            memset( dst, 0, XBOX_AUDIO_BYTES );
        xbox_write = ( xbox_write + 1 ) % XBOX_AUDIO_BUFFERS;
        xbox_filled++;
    }
}
