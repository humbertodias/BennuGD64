/*
 * PlayStation 2 audio. Compiled only into the ps2-mips build.
 *
 * Do not call init_audio_driver() here: SDL's ps2 audio bootstrap already
 * does, and a second call makes that bootstrap fail so SDL picks dummy
 * (silent). Songs/SFX live in the DCB xfile — slurp them so MIX_LoadAudio_IO
 * never seeks USB from the audio thread (SDL_IOFromBGDFP is silent here).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "files.h"
#include "mod_sound_ps2.h"

typedef struct Ps2Slurp
{
    Uint8 * data;
    size_t  size;
    size_t  pos;
} Ps2Slurp;

int modsound_ps2_skip_audio( void )
{
    return 0;
}

int modsound_ps2_skip_song( void )
{
    return 0;
}

void modsound_ps2_adjust_rate( int * audio_rate )
{
    if ( audio_rate && *audio_rate > 22050 )
        *audio_rate = 22050;
}

void modsound_ps2_prepare( void )
{
    static int once;

    if ( once )
        return;
    once = 1;

    SDL_SetHintWithPriority( SDL_HINT_AUDIO_DRIVER, "ps2", SDL_HINT_OVERRIDE );
    if ( !SDL_WasInit( SDL_INIT_AUDIO ) )
        SDL_InitSubSystem( SDL_INIT_AUDIO );
}

static Sint64 SDLCALL slurp_size( void * ud )
{
    return ( Sint64 )( ( Ps2Slurp * ) ud )->size;
}

static Sint64 SDLCALL slurp_seek( void * ud, Sint64 offset, SDL_IOWhence whence )
{
    Ps2Slurp * s = ( Ps2Slurp * ) ud;
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
    Ps2Slurp * s = ( Ps2Slurp * ) ud;
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
    Ps2Slurp * s = ( Ps2Slurp * ) ud;

    if ( s )
    {
        free( s->data );
        free( s );
    }
    return true;
}

SDL_IOStream * modsound_ps2_slurp_file( file * fp )
{
    Ps2Slurp * s;
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

    s = ( Ps2Slurp * ) calloc( 1, sizeof( *s ) );
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
