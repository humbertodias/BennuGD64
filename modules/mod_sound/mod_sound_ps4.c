/*
 * PlayStation 4 audio. Compiled only into the ps4-x86_64 build.
 *
 * SDL_mixer generates samples and a native pthread feeds them to AudioOut.
 * Songs/SFX on USB are slurped so decoding never seeks from the audio thread.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <orbis/AudioOut.h>
#include <orbis/UserService.h>

#include "files.h"
#include "mod_sound_ps4.h"

typedef struct Ps4Slurp
{
    Uint8 * data;
    size_t  size;
    size_t  pos;
} Ps4Slurp;

enum
{
    PS4_AUDIO_FRAMES = 1024,
    PS4_AUDIO_CHANNELS = 2,
    PS4_AUDIO_BUFFERS = 2
};

static pthread_t ps4_audio_thread;
static int ps4_audio_thread_started;
static MIX_Mixer * ps4_audio_mixer;
static volatile int ps4_audio_running;
static int ps4_audio_handle = -1;
static float * ps4_audio_buffers[ PS4_AUDIO_BUFFERS ];

void modsound_ps4_prepare( void )
{
    /* AudioOut is driven directly; upstream SDL3 has no Orbis audio driver. */
}

void modsound_ps4_adjust_spec( SDL_AudioSpec * spec )
{
    if ( !spec )
        return;
    spec->freq = 48000;
    spec->format = SDL_AUDIO_F32;
    spec->channels = PS4_AUDIO_CHANNELS;
}

static void * ps4_audio_worker( void * unused )
{
    const int bytes = PS4_AUDIO_FRAMES * PS4_AUDIO_CHANNELS * ( int ) sizeof( float );
    int current = 0;

    ( void ) unused;
    fprintf( stderr, "bgdi: PS4 AudioOut worker started\n" );
    while ( ps4_audio_running )
    {
        if ( MIX_Generate( ps4_audio_mixer, ps4_audio_buffers[ current ], bytes ) < 0 )
        {
            fprintf( stderr, "bgdi: MIX_Generate failed: %s\n", SDL_GetError() );
            memset( ps4_audio_buffers[ current ], 0, ( size_t ) bytes );
        }
        if ( sceAudioOutOutput( ps4_audio_handle, ps4_audio_buffers[ current ] ) < 0 )
        {
            fprintf( stderr, "bgdi: sceAudioOutOutput failed\n" );
            break;
        }
        current = ( current + 1 ) % PS4_AUDIO_BUFFERS;
    }
    fprintf( stderr, "bgdi: PS4 AudioOut worker stopped\n" );
    return NULL;
}

void modsound_ps4_stop_output( void )
{
    int i;

    ps4_audio_running = 0;
    if ( ps4_audio_thread_started )
    {
        pthread_join( ps4_audio_thread, NULL );
        ps4_audio_thread_started = 0;
    }
    if ( ps4_audio_handle >= 0 )
    {
        sceAudioOutClose( ps4_audio_handle );
        ps4_audio_handle = -1;
    }
    for ( i = 0; i < PS4_AUDIO_BUFFERS; i++ )
    {
        free( ps4_audio_buffers[ i ] );
        ps4_audio_buffers[ i ] = NULL;
    }
    ps4_audio_mixer = NULL;
}

int modsound_ps4_start_output( MIX_Mixer * mixer )
{
    const size_t bytes = PS4_AUDIO_FRAMES * PS4_AUDIO_CHANNELS * sizeof( float );
    int i, rc;

    if ( !mixer )
        return -1;
    if ( ps4_audio_thread_started )
        return 0;

    rc = sceAudioOutInit();
    fprintf( stderr, "bgdi: sceAudioOutInit=%d\n", rc );
    if ( rc != 0 && ( uint32_t ) rc != ORBIS_AUDIO_OUT_ERROR_ALREADY_INIT )
        return rc;

    ps4_audio_handle = sceAudioOutOpen( ORBIS_USER_SERVICE_USER_ID_SYSTEM,
                                        ORBIS_AUDIO_OUT_PORT_TYPE_MAIN, 0,
                                        PS4_AUDIO_FRAMES, 48000,
                                        ORBIS_AUDIO_OUT_PARAM_FORMAT_FLOAT_STEREO );
    fprintf( stderr, "bgdi: sceAudioOutOpen=%d\n", ps4_audio_handle );
    if ( ps4_audio_handle < 0 )
        return ps4_audio_handle;

    for ( i = 0; i < PS4_AUDIO_BUFFERS; i++ )
    {
        ps4_audio_buffers[ i ] = ( float * ) memalign( 64, bytes );
        if ( !ps4_audio_buffers[ i ] )
        {
            modsound_ps4_stop_output();
            return -1;
        }
        memset( ps4_audio_buffers[ i ], 0, bytes );
    }

    ps4_audio_mixer = mixer;
    ps4_audio_running = 1;
    rc = pthread_create( &ps4_audio_thread, NULL, ps4_audio_worker, NULL );
    if ( rc != 0 )
    {
        fprintf( stderr, "bgdi: pthread_create(audio) failed: %d\n", rc );
        modsound_ps4_stop_output();
        return -1;
    }
    ps4_audio_thread_started = 1;
    return 0;
}

static Sint64 SDLCALL slurp_size( void * ud )
{
    return ( Sint64 )( ( Ps4Slurp * ) ud )->size;
}

static Sint64 SDLCALL slurp_seek( void * ud, Sint64 offset, SDL_IOWhence whence )
{
    Ps4Slurp * s = ( Ps4Slurp * ) ud;
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
    Ps4Slurp * s = ( Ps4Slurp * ) ud;
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
    Ps4Slurp * s = ( Ps4Slurp * ) ud;

    if ( s )
    {
        free( s->data );
        free( s );
    }
    return true;
}

SDL_IOStream * modsound_ps4_slurp_file( file * fp )
{
    Ps4Slurp * s;
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

    s = ( Ps4Slurp * ) calloc( 1, sizeof( *s ) );
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
