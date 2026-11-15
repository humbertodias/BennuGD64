/*
 * PlayStation 5 audio. Compiled only into the ps5-x86_64 build.
 *
 * The default build keeps the mixer exports and slurps songs into memory,
 * but does not open sceAudioOut (same policy as the PS4 dummy-audio build).
 */

#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "files.h"
#include "mod_sound_ps5.h"

typedef struct Ps5Slurp
{
    Uint8 * data;
    size_t  size;
    size_t  pos;
} Ps5Slurp;

void modsound_ps5_prepare( void )
{
}

void modsound_ps5_adjust_spec( SDL_AudioSpec * spec )
{
    if ( !spec )
        return;
    spec->freq = 48000;
    spec->format = SDL_AUDIO_F32;
    spec->channels = 2;
}

void modsound_ps5_stop_output( void )
{
}

int modsound_ps5_start_output( MIX_Mixer * mixer )
{
    ( void ) mixer;
    return 0;
}

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
