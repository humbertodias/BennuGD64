/*
 * PlayStation 5 audio. Compiled only into the ps5-x86_64 build.
 *
 * SDL_mixer generates samples for sceAudioOut. Audio files are read into
 * memory before playback so decoding can run on the audio thread.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "files.h"
#include "mod_sound_ps5.h"
#include "ps5_sce.h"

typedef struct Ps5Slurp
{
    Uint8 * data;
    size_t  size;
    size_t  pos;
} Ps5Slurp;

enum
{
    PS5_AUDIO_RATE = 48000,
    PS5_AUDIO_CHANNELS = 2,
    PS5_AUDIO_FRAMES = 1024
};

void modsound_ps5_adjust_spec( SDL_AudioSpec * spec )
{
    spec->freq = PS5_AUDIO_RATE;
    spec->channels = PS5_AUDIO_CHANNELS;
#ifdef TARGET_LIBRETRO
    spec->format = SDL_AUDIO_S16;
#else
    spec->format = SDL_AUDIO_F32;
#endif
}

#ifndef TARGET_LIBRETRO
static SDL_Thread * ps5_audio_thread;
static SDL_AtomicInt ps5_audio_running;
static int ps5_audio_handle = -1;
static _Alignas(64) float ps5_audio_buffers[2][PS5_AUDIO_FRAMES * PS5_AUDIO_CHANNELS];

static int SDLCALL ps5_audio_worker( void * userdata )
{
    MIX_Mixer * mixer = userdata;
    int current = 0;
    bool mix_error_logged = false;

    while ( SDL_GetAtomicInt( &ps5_audio_running ) )
    {
        float * samples = ps5_audio_buffers[current];
        const int bytes = sizeof( ps5_audio_buffers[current] );
        if ( MIX_Generate( mixer, samples, bytes ) < 0 )
        {
            if ( !mix_error_logged )
                fprintf( stderr, "[SOUND] PS5 MIX_Generate: %s\n", SDL_GetError() );
            mix_error_logged = true;
            memset( samples, 0, bytes );
        }

        int result = sceAudioOutOutput( ps5_audio_handle, samples );
        if ( result < 0 )
        {
            fprintf( stderr, "[SOUND] sceAudioOutOutput: %#x\n", ( unsigned int ) result );
            return result;
        }
        current ^= 1;
    }
    return 0;
}

void modsound_ps5_stop_output( void )
{
    SDL_SetAtomicInt( &ps5_audio_running, 0 );
    if ( ps5_audio_thread )
    {
        SDL_WaitThread( ps5_audio_thread, NULL );
        ps5_audio_thread = NULL;
    }
    if ( ps5_audio_handle >= 0 )
    {
        sceAudioOutClose( ps5_audio_handle );
        ps5_audio_handle = -1;
    }
}

int modsound_ps5_start_output( MIX_Mixer * mixer )
{
    int result = sceAudioOutInit();
    if ( result != 0 && ( uint32_t ) result != SCE_AUDIO_OUT_ERROR_ALREADY_INIT )
    {
        fprintf( stderr, "[SOUND] sceAudioOutInit: %#x\n", ( unsigned int ) result );
        return result;
    }

    result = sceAudioOutOpen( SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_AUDIO_OUT_PORT_TYPE_MAIN, 0,
                              PS5_AUDIO_FRAMES, PS5_AUDIO_RATE, SCE_AUDIO_OUT_FORMAT_FLOAT_STEREO );
    if ( result < 0 )
    {
        fprintf( stderr, "[SOUND] sceAudioOutOpen: %#x\n", ( unsigned int ) result );
        return result;
    }
    ps5_audio_handle = result;

    SDL_SetAtomicInt( &ps5_audio_running, 1 );
    ps5_audio_thread = SDL_CreateThread( ps5_audio_worker, "BennuGD PS5 Audio", mixer );
    if ( !ps5_audio_thread )
    {
        fprintf( stderr, "[SOUND] PS5 SDL_CreateThread: %s\n", SDL_GetError() );
        modsound_ps5_stop_output();
        return -1;
    }
    return 0;
}
#endif

static Sint64 SDLCALL slurp_size( void * ud )
{
    return ( Sint64 )( ( Ps5Slurp * ) ud )->size;
}

static Sint64 SDLCALL slurp_seek( void * ud, Sint64 offset, SDL_IOWhence whence )
{
    Ps5Slurp * s = ( Ps5Slurp * ) ud;
    Sint64 pos = ( Sint64 ) s->pos;

    if ( whence == SDL_IO_SEEK_CUR )
        pos += offset;
    else if ( whence == SDL_IO_SEEK_END )
        pos = ( Sint64 ) s->size + offset;
    else
        pos = offset;
    if ( pos < 0 || ( size_t ) pos > s->size )
        return -1;
    s->pos = ( size_t ) pos;
    return pos;
}

static size_t SDLCALL slurp_read( void * ud, void * ptr, size_t size, SDL_IOStatus * status )
{
    Ps5Slurp * s = ( Ps5Slurp * ) ud;
    size_t left = s->size - s->pos;

    if ( size > left )
        size = left;
    if ( size == 0 )
    {
        if ( status )
            *status = SDL_IO_STATUS_EOF;
        return 0;
    }
    memcpy( ptr, s->data + s->pos, size );
    s->pos += size;
    return size;
}

static bool SDLCALL slurp_close( void * ud )
{
    Ps5Slurp * s = ( Ps5Slurp * ) ud;

    if ( s )
    {
        free( s->data );
        free( s );
    }
    return true;
}

SDL_IOStream * modsound_ps5_slurp_file( file * fp )
{
    Ps5Slurp * s;
    SDL_IOStreamInterface iface;
    SDL_IOStream * io;
    int n, got;

    if ( !fp )
        return NULL;
    n = file_size( fp );
    if ( n < 1 )
    {
        file_close( fp );
        return NULL;
    }

    s = ( Ps5Slurp * ) calloc( 1, sizeof( *s ) );
    if ( !s )
    {
        file_close( fp );
        return NULL;
    }
    s->data = ( Uint8 * ) malloc( ( size_t ) n );
    if ( !s->data )
    {
        free( s );
        file_close( fp );
        return NULL;
    }

    file_seek( fp, 0, SEEK_SET );
    got = file_read( fp, s->data, n );
    file_close( fp );
    if ( got != n )
    {
        free( s->data );
        free( s );
        return NULL;
    }
    s->size = ( size_t ) n;

    SDL_INIT_INTERFACE( &iface );
    iface.size  = slurp_size;
    iface.seek  = slurp_seek;
    iface.read  = slurp_read;
    iface.close = slurp_close;
    io = SDL_OpenIO( &iface, s );
    if ( !io )
    {
        free( s->data );
        free( s );
        return NULL;
    }
    return io;
}
