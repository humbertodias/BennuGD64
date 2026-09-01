/*
 * PlayStation 3 audio. Compiled only into the ps3-ppu build.
 *
 * libaudio is F32BE at 48 kHz. MIX_CreateMixerDevice with S16 (Bennu
 * default) makes PS3AUDIO_OpenDevice return "Unsupported audio format"
 * because ClosestAudioFormats never hits F32BE. Force the ps3 driver
 * before SDL_INIT_AUDIO so dummy is not picked. Songs/SFX on USB: slurp
 * so MIX_LoadAudio_IO never seeks from the audio thread.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "files.h"
#include "mod_sound_ps3.h"

typedef struct Ps3Slurp
{
    Uint8 * data;
    size_t  size;
    size_t  pos;
} Ps3Slurp;

void modsound_ps3_prepare( void )
{
    static int once;

    if ( once )
        return;
    once = 1;

    SDL_SetHintWithPriority( SDL_HINT_AUDIO_DRIVER, "ps3", SDL_HINT_OVERRIDE );
    if ( !SDL_WasInit( SDL_INIT_AUDIO ) )
        SDL_InitSubSystem( SDL_INIT_AUDIO );
}

void modsound_ps3_adjust_spec( SDL_AudioSpec * spec )
{
    if ( !spec )
        return;
    spec->freq = 48000;
    spec->format = SDL_AUDIO_F32;
}

static Sint64 SDLCALL slurp_size( void * ud )
{
    return ( Sint64 )( ( Ps3Slurp * ) ud )->size;
}

static Sint64 SDLCALL slurp_seek( void * ud, Sint64 offset, SDL_IOWhence whence )
{
    Ps3Slurp * s = ( Ps3Slurp * ) ud;
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
    Ps3Slurp * s = ( Ps3Slurp * ) ud;
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
    Ps3Slurp * s = ( Ps3Slurp * ) ud;

    if ( s )
    {
        free( s->data );
        free( s );
    }
    return true;
}

SDL_IOStream * modsound_ps3_slurp_file( file * fp )
{
    Ps3Slurp * s;
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

    s = ( Ps3Slurp * ) calloc( 1, sizeof( *s ) );
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
