/*
 *  Copyright ? 2006-2013 SplinterGU (Fenix/Bennugd)
 *  Copyright ? 2002-2006 Fenix Team (Fenix)
 *  Copyright ? 1999-2002 Jos? Luis Cebri?n Pag?e (Fenix)
 *
 *  This file is part of Bennu - Game Development
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

#pragma comment (lib, "SDL3_mixer")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bgddl.h"
#include "bgd_handles.h"

#include <SDL3/SDL.h>
#include "sdl3_compat.h"

#include <SDL3_mixer/SDL_mixer.h>

#include "files.h"
#include "xstrings.h"

#include "dlvaracc.h"

#include "bgload.h"

/* --------------------------------------------------------------------------- */

#define MAX_SOUND_CHANNELS  32

typedef struct SoundAudio {
    MIX_Audio * audio;
    float       gain;   /* 0.0 .. 1.0 (from Mix volume 0..128) */
} SoundAudio;

static int audio_initialized = 0 ;
static MIX_Mixer * mixer = NULL;
static MIX_Track * music_track = NULL;
static MIX_Track * channels[MAX_SOUND_CHANNELS];
static float channel_gain[MAX_SOUND_CHANNELS];
static float music_gain = 1.0f;
static int num_channels = 0;
static int reserved_channels = 0;

/* --------------------------------------------------------------------------- */

#define SOUND_FREQ              0
#define SOUND_MODE              1
#define SOUND_CHANNELS          2

/* --------------------------------------------------------------------------- */
/* Son las variables que se desea acceder.                                     */
/* El interprete completa esta estructura, si la variable existe.              */
/* (usada en tiempo de ejecucion)                                              */

DLVARFIXUP  __bgdexport( mod_sound, globals_fixup )[] =
{
    /* Nombre de variable global, puntero al dato, tama?o del elemento, cantidad de elementos */
    { "sound_freq", NULL, -1, -1 },
    { "sound_mode", NULL, -1, -1 },
    { "sound_channels", NULL, -1, -1 },
    { NULL, NULL, -1, -1 }
};

/* ------------------------------------- */
/* Interfaz SDL_IOStream Bennu           */
/* ------------------------------------- */

static Sint64 SDLCALL __modsound_size_cb( void *userdata )
{
    return ( Sint64 ) file_size( ( file * ) userdata );
}

static Sint64 SDLCALL __modsound_seek_cb( void *userdata, Sint64 offset, SDL_IOWhence whence )
{
    if ( file_seek( ( file * ) userdata, ( int ) offset, ( int ) whence ) < 0 ) return ( -1 );
    return ( Sint64 ) file_pos( ( file * ) userdata );
}

static size_t SDLCALL __modsound_read_cb( void *userdata, void *ptr, size_t size, SDL_IOStatus *status )
{
    int ret = file_read( ( file * ) userdata, ptr, ( int ) size );
    if ( ret < 0 )
    {
        if ( status ) *status = SDL_IO_STATUS_ERROR;
        return 0;
    }
    if ( ret == 0 && file_eof( ( file * ) userdata ) )
    {
        if ( status ) *status = SDL_IO_STATUS_EOF;
    }
    return ( size_t ) ret;
}

static size_t SDLCALL __modsound_write_cb( void *userdata, const void *ptr, size_t size, SDL_IOStatus *status )
{
    int ret = file_write( ( file * ) userdata, ( void * ) ptr, ( int ) size );
    if ( ret < 0 )
    {
        if ( status ) *status = SDL_IO_STATUS_ERROR;
        return 0;
    }
    return ( size_t ) ret;
}

static bool SDLCALL __modsound_close_cb( void *userdata )
{
    if ( userdata ) file_close( ( file * ) userdata );
    return true;
}

static SDL_IOStream *SDL_IOFromBGDFP( file *fp )
{
    SDL_IOStreamInterface iface;
    SDL_INIT_INTERFACE( &iface );
    iface.size  = __modsound_size_cb;
    iface.seek  = __modsound_seek_cb;
    iface.read  = __modsound_read_cb;
    iface.write = __modsound_write_cb;
    iface.close = __modsound_close_cb;
    return SDL_OpenIO( &iface, fp );
}

static float volume_to_gain( int volume )
{
    if ( volume < 0 ) volume = 0;
    if ( volume > 128 ) volume = 128;
    return ( float ) volume / 128.0f;
}

static SoundAudio * sound_audio_new( MIX_Audio * audio )
{
    SoundAudio * sa;
    if ( !audio ) return NULL;
    sa = ( SoundAudio * ) SDL_calloc( 1, sizeof( SoundAudio ) );
    if ( !sa )
    {
        MIX_DestroyAudio( audio );
        return NULL;
    }
    sa->audio = audio;
    sa->gain = 1.0f;
    return sa;
}

static void sound_audio_free( SoundAudio * sa )
{
    if ( !sa ) return;
    if ( sa->audio ) MIX_DestroyAudio( sa->audio );
    SDL_free( sa );
}

static SDL_PropertiesID play_props_with_loops( int loops )
{
    SDL_PropertiesID props = SDL_CreateProperties();
    if ( props ) SDL_SetNumberProperty( props, MIX_PROP_PLAY_LOOPS_NUMBER, loops );
    return props;
}

static int find_free_channel( void )
{
    int i;
    for ( i = reserved_channels; i < num_channels; i++ )
    {
        if ( channels[i] && !MIX_TrackPlaying( channels[i] ) ) return i;
    }
    return -1;
}

/* --------------------------------------------------------------------------- */

/*
 *  FUNCTION : sound_init
 *
 *  Set the SDL_Mixer library
 *
 *  PARAMS:
 *      no params
 *
 *  RETURN VALUE:
 *
 *  no return
 *
 */

static int sound_init()
{
    int audio_rate;
    int audio_channels;
    int i;
    SDL_AudioSpec spec;

    if ( audio_initialized ) return 0;

    if ( !MIX_Init() )
    {
        fprintf( stderr, "[SOUND] No se pudo inicializar el audio: %s\n", SDL_GetError() );
        return -1;
    }

    /* Initialize variables: but limit quality to some fixed options */
    audio_rate = GLODWORD( mod_sound, SOUND_FREQ );

    if ( audio_rate > 22050 )
        audio_rate = 44100;
    else if ( audio_rate > 11025 )
        audio_rate = 22050;
    else
        audio_rate = 11025;

    audio_channels = GLODWORD( mod_sound, SOUND_MODE ) + 1;

    SDL_zero( spec );
    spec.freq = audio_rate;
    spec.format = SDL_AUDIO_S16;
    spec.channels = audio_channels;

    mixer = MIX_CreateMixerDevice( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec );
    if ( !mixer )
    {
        fprintf( stderr, "[SOUND] No se pudo inicializar el audio: %s\n", SDL_GetError() );
        MIX_Quit();
        audio_initialized = 0;
        return -1;
    }

    num_channels = ( int ) GLODWORD( mod_sound, SOUND_CHANNELS );
    if ( num_channels <= 0 ) num_channels = 8;
    if ( num_channels > MAX_SOUND_CHANNELS ) num_channels = MAX_SOUND_CHANNELS;
    GLODWORD( mod_sound, SOUND_CHANNELS ) = num_channels;

    music_track = MIX_CreateTrack( mixer );
    if ( !music_track )
    {
        fprintf( stderr, "[SOUND] No se pudo crear music track: %s\n", SDL_GetError() );
        MIX_DestroyMixer( mixer );
        mixer = NULL;
        MIX_Quit();
        return -1;
    }
    MIX_SetTrackGain( music_track, music_gain );

    for ( i = 0; i < num_channels; i++ )
    {
        channels[i] = MIX_CreateTrack( mixer );
        channel_gain[i] = 1.0f;
        if ( !channels[i] )
        {
            fprintf( stderr, "[SOUND] No se pudo crear channel track %d: %s\n", i, SDL_GetError() );
        }
        else
        {
            MIX_SetTrackGain( channels[i], channel_gain[i] );
        }
    }

    audio_initialized = 1;
    return 0;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : sound_close
 *
 *  Close all the audio set
 *
 *  PARAMS:
 *      no params
 *
 *  RETURN VALUE:
 *
 *  no return
 *
 */

static void sound_close()
{
    int i;

    if ( !audio_initialized ) return;

    if ( music_track )
    {
        MIX_StopTrack( music_track, 0 );
        MIX_DestroyTrack( music_track );
        music_track = NULL;
    }

    for ( i = 0; i < num_channels; i++ )
    {
        if ( channels[i] )
        {
            MIX_StopTrack( channels[i], 0 );
            MIX_DestroyTrack( channels[i] );
            channels[i] = NULL;
        }
    }

    if ( mixer )
    {
        MIX_DestroyMixer( mixer );
        mixer = NULL;
    }

    MIX_Quit();

    num_channels = 0;
    reserved_channels = 0;
    audio_initialized = 0;
}


/* ------------------ */
/* Sonido MOD y OGG   */
/* ------------------ */

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : load_song
 *
 *  Load a MOD/OGG from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE:
 *
 *  mod pointer
 *
 */

static int load_song( const char * filename )
{
    MIX_Audio * music = NULL;
    SoundAudio * sa;
    file      *fp;
    SDL_IOStream *io;

    if ( !audio_initialized && sound_init() ) return ( 0 );

    if ( !( fp = file_open( filename, "rb0" ) ) ) return ( 0 );

    io = SDL_IOFromBGDFP( fp );
    if ( !io )
    {
        file_close( fp );
        return ( 0 );
    }

    /* closeio=true: IOStream close callback closes the Bennu file */
    if ( !( music = MIX_LoadAudio_IO( mixer, io, false, true ) ) )
    {
        fprintf( stderr, "Couldn't load %s: %s\n", filename, SDL_GetError() );
        return( 0 );
    }

    sa = sound_audio_new( music );
    if ( !sa ) return ( 0 );

    return bgd_handle_put( sa );
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : play_song
 *
 *  Play a MOD/OGG
 *
 *  PARAMS:
 *      mod pointer
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int play_song( int id, int loops )
{
    SoundAudio * sa = ( SoundAudio * ) bgd_handle_get( id );
    if ( audio_initialized && sa && sa->audio && music_track )
    {
        SDL_PropertiesID props;
        bool ok;

        if ( !MIX_SetTrackAudio( music_track, sa->audio ) )
        {
            fprintf( stderr, "%s", SDL_GetError() );
            return -1;
        }
        MIX_SetTrackGain( music_track, music_gain * sa->gain );

        props = play_props_with_loops( loops );
        ok = MIX_PlayTrack( music_track, props );
        if ( props ) SDL_DestroyProperties( props );
        if ( !ok )
        {
            fprintf( stderr, "%s", SDL_GetError() );
            return -1;
        }
        return 0;
    }

    fprintf( stderr, "Play song called with invalid handle" );
    return( -1 );
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : fade_music_in
 *
 *  Play a MOD/OGG fading in it
 *
 *  PARAMS:
 *      mod pointer
 *      number of loops (-1 infinite loops)
 *      ms  microsends of fadding
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int fade_music_in( int id, int loops, int ms )
{
    SoundAudio * sa = ( SoundAudio * ) bgd_handle_get( id );
    if ( audio_initialized && sa && sa->audio && music_track )
    {
        SDL_PropertiesID props;
        bool ok;

        if ( !MIX_SetTrackAudio( music_track, sa->audio ) ) return -1;
        MIX_SetTrackGain( music_track, music_gain * sa->gain );

        props = play_props_with_loops( loops );
        if ( props && ms > 0 )
            SDL_SetNumberProperty( props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, ms );
        ok = MIX_PlayTrack( music_track, props );
        if ( props ) SDL_DestroyProperties( props );
        return ok ? 0 : -1;
    }
    return( -1 );
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : fade_music_off
 *
 *  Stop the play of a mod
 *
 *  PARAMS:
 *
 *  ms  microsends of fadding
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int fade_music_off( int ms )
{
    Sint64 frames = 0;
    if ( !audio_initialized || !music_track ) return ( 0 );
    if ( ms > 0 ) frames = MIX_TrackMSToFrames( music_track, ( Sint64 ) ms );
    if ( frames < 0 ) frames = 0;
    return MIX_StopTrack( music_track, frames ) ? 1 : 0;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : unload_song
 *
 *  Play a MOD
 *
 *  PARAMS:
 *
 *  mod id
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int unload_song( int id )
{
    SoundAudio * sa = ( SoundAudio * ) bgd_handle_get( id );
    if ( audio_initialized && sa )
    {
        if ( music_track && MIX_TrackPlaying( music_track ) && MIX_GetTrackAudio( music_track ) == sa->audio )
            MIX_StopTrack( music_track, 0 );
        sound_audio_free( sa );
        bgd_handle_free( id );
    }
    return ( 0 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : stop_song
 *
 *  Stop the play of a mod
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int stop_song( void )
{
    if ( audio_initialized && music_track ) MIX_StopTrack( music_track, 0 );
    return ( 0 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : pause_song
 *
 *  Pause the mod in curse, you can resume it after
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int pause_song( void )
{
    if ( audio_initialized && music_track ) MIX_PauseTrack( music_track );
    return ( 0 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : resume_song
 *
 *  Resume the mod, paused before
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int resume_song( void )
{
    if ( audio_initialized && music_track ) MIX_ResumeTrack( music_track );
    return( 0 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : is_playing_song
 *
 *  Check if there is any mod playing
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *  TRUE OR FALSE if there is no error
 *
 */

static int is_playing_song( void )
{
    if ( !audio_initialized || !music_track ) return ( 0 );
    return MIX_TrackPlaying( music_track ) ? 1 : 0;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : set_song_volume
 *
 *  Set the volume for mod playing (0-128)
 *
 *  PARAMS:
 *
 *  int volume
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *  0 if there is no error
 *
 */

static int set_song_volume( int volume )
{
    if ( !audio_initialized && sound_init() ) return ( -1 );

    music_gain = volume_to_gain( volume );
    if ( music_track ) MIX_SetTrackGain( music_track, music_gain );
    return 0;
}

/* ------------ */
/* Sonido WAV   */
/* ------------ */

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : load_wav
 *
 *  Load a WAV from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE:
 *
 *  wav pointer
 *
 */

static int load_wav( const char * filename )
{
    MIX_Audio * music = NULL;
    SoundAudio * sa;
    file      *fp;
    SDL_IOStream *io;

    if ( !audio_initialized && sound_init() ) return ( 0 );

    if ( !( fp = file_open( filename, "rb0" ) ) ) return ( 0 );

    io = SDL_IOFromBGDFP( fp );
    if ( !io )
    {
        file_close( fp );
        return ( 0 );
    }

    if ( !( music = MIX_LoadAudio_IO( mixer, io, true, true ) ) )
    {
        fprintf( stderr, "Couldn't load %s: %s\n", filename, SDL_GetError() );
        return( 0 );
    }

    sa = sound_audio_new( music );
    if ( !sa ) return ( 0 );

    return bgd_handle_put( sa );
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : play_wav
 *
 *  Play a WAV
 *
 *  PARAMS:
 *      wav pointer;
 *      number of loops (-1 infinite loops)
 *      channel (-1 any channel)
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *  else channel where the music plays
 *
 */

static int play_wav( int id, int loops, int channel )
{
    SoundAudio * sa = ( SoundAudio * ) bgd_handle_get( id );
    SDL_PropertiesID props;
    bool ok;

    if ( !audio_initialized || !sa || !sa->audio ) return ( -1 );

    if ( channel < 0 ) channel = find_free_channel();
    if ( channel < 0 || channel >= num_channels || !channels[channel] ) return ( -1 );

    if ( !MIX_SetTrackAudio( channels[channel], sa->audio ) ) return ( -1 );
    MIX_SetTrackGain( channels[channel], channel_gain[channel] * sa->gain );

    props = play_props_with_loops( loops );
    ok = MIX_PlayTrack( channels[channel], props );
    if ( props ) SDL_DestroyProperties( props );
    return ok ? channel : -1;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : unload_wav
 *
 *  Frees the resources from a wav, unloading it
 *
 *  PARAMS:
 *
 *  wav pointer
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int unload_wav( int id )
{
    SoundAudio * sa = ( SoundAudio * ) bgd_handle_get( id );
    if ( audio_initialized && sa )
    {
        sound_audio_free( sa );
        bgd_handle_free( id );
    }
    return ( 0 );
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : stop_wav
 *
 *  Stop a wav playing
 *
 *  PARAMS:
 *
 *  int channel
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int stop_wav( int canal )
{
    int i;
    if ( !audio_initialized ) return ( -1 );

    if ( canal == -1 )
    {
        for ( i = 0; i < num_channels; i++ )
            if ( channels[i] ) MIX_StopTrack( channels[i], 0 );
        return 0;
    }

    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );
    if ( MIX_TrackPlaying( channels[canal] ) )
        return MIX_StopTrack( channels[canal], 0 ) ? 0 : -1;
    return ( -1 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : pause_wav
 *
 *  Pause a wav playing, you can resume it after
 *
 *  PARAMS:
 *
 *  int channel
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int pause_wav( int canal )
{
    int i;
    if ( !audio_initialized ) return ( -1 );

    if ( canal == -1 )
    {
        for ( i = 0; i < num_channels; i++ )
            if ( channels[i] ) MIX_PauseTrack( channels[i] );
        return 0;
    }

    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );
    if ( MIX_TrackPlaying( channels[canal] ) )
    {
        MIX_PauseTrack( channels[canal] );
        return ( 0 ) ;
    }
    return ( -1 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : resume_wav
 *
 *  Resume a wav playing, paused before
 *
 *  PARAMS:
 *
 *  int channel
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int resume_wav( int canal )
{
    int i;
    if ( !audio_initialized ) return ( -1 );

    if ( canal == -1 )
    {
        for ( i = 0; i < num_channels; i++ )
            if ( channels[i] ) MIX_ResumeTrack( channels[i] );
        return 0;
    }

    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );
    MIX_ResumeTrack( channels[canal] );
    return ( 0 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : is_playing_wav
 *
 *  Check a wav playing
 *
 *  PARAMS:
 *
 *  int channel
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *  TRUE OR FALSE if there is no error
 *
 */

static int is_playing_wav( int canal )
{
    int i, n = 0;
    if ( !audio_initialized ) return ( 0 );

    if ( canal == -1 )
    {
        for ( i = 0; i < num_channels; i++ )
            if ( channels[i] && MIX_TrackPlaying( channels[i] ) ) n++;
        return n;
    }

    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( 0 );
    return MIX_TrackPlaying( channels[canal] ) ? 1 : 0;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : set_wav_volume
 *
 *  Set the volume for wav playing (0-128) IN SAMPLE
 *
 *  PARAMS:
 *
 *  channel id
 *  int volume
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int  set_wav_volume( int sample, int volume )
{
    int prev;
    SoundAudio * sa;

    if ( !audio_initialized ) return ( -1 );

    if ( volume < 0 ) volume = 0;
    if ( volume > 128 ) volume = 128;

    if ( sample )
    {
        sa = ( SoundAudio * ) bgd_handle_get( sample );
        if ( sa )
        {
            prev = ( int ) ( sa->gain * 128.0f + 0.5f );
            sa->gain = volume_to_gain( volume );
            return prev;
        }
    }

    return -1 ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : set_channel_volume
 *
 *  Set the volume for wav playing (0-128) IN CHANNEL
 *
 *  PARAMS:
 *
 *  channel id
 *  int volume
 *
 *  RETURN VALUE:
 *
 * -1 if there is any error
 *
 */

static int  set_channel_volume( int canal, int volume )
{
    int i, prev;

    if ( !audio_initialized && sound_init() ) return ( -1 );

    if ( volume < 0 ) volume = 0;
    if ( volume > 128 ) volume = 128;

    if ( canal == -1 )
    {
        prev = 0;
        for ( i = 0; i < num_channels; i++ )
        {
            channel_gain[i] = volume_to_gain( volume );
            if ( channels[i] ) MIX_SetTrackGain( channels[i], channel_gain[i] );
        }
        return volume;
    }

    if ( canal < 0 || canal >= num_channels ) return ( -1 );
    prev = ( int ) ( channel_gain[canal] * 128.0f + 0.5f );
    channel_gain[canal] = volume_to_gain( volume );
    if ( channels[canal] ) MIX_SetTrackGain( channels[canal], channel_gain[canal] );
    return prev;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : reserve_channels
 *
 *  Reserve the first channels (0 -> n-1) for the application, i.e. don't allocate
 *  them dynamically to the next sample if requested with a -1 value below.
 *
 *  PARAMS:
 *  number of channels to reserve.
 *
 *  RETURN VALUE:
 *  number of reserved channels.
 * -1 if there is any error
 *
 */

static int reserve_channels( int canales )
{
    if ( !audio_initialized && sound_init() ) return ( -1 );
    if ( canales < 0 ) canales = 0;
    if ( canales > num_channels ) canales = num_channels;
    reserved_channels = canales;
    return reserved_channels;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : set_panning
 *
 *  Set the panning for a wav channel
 *
 *  PARAMS:
 *
 *  channel
 *  left volume (0-255)
 *  right volume (0-255)
 *
 */

static int set_panning( int canal, int left, int right )
{
    MIX_StereoGains gains;

    if ( !audio_initialized && sound_init() ) return ( -1 );
    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );

    if ( MIX_TrackPlaying( channels[canal] ) )
    {
        if ( left < 0 ) left = 0;
        if ( right < 0 ) right = 0;
        if ( left > 255 ) left = 255;
        if ( right > 255 ) right = 255;
        gains.left = ( float ) left / 255.0f;
        gains.right = ( float ) right / 255.0f;
        MIX_SetTrackStereo( channels[canal], &gains );
        return ( 0 ) ;
    }
    return ( -1 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : set_position
 *
 *  Set the position of a channel. (angle) is an integer from 0 to 360
 *
 *  PARAMS:
 *
 *  channel
 *  angle (0-360)
 *  distance (0-255)
 *
 */

static int set_position( int canal, int angle, int dist )
{
    MIX_Point3D pos;
    float rad, d;

    if ( !audio_initialized && sound_init() ) return ( -1 );
    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );

    if ( MIX_TrackPlaying( channels[canal] ) )
    {
        if ( dist < 0 ) dist = 0;
        if ( dist > 255 ) dist = 255;
        d = ( float ) dist / 255.0f;
        rad = ( float ) angle * ( float )( 3.14159265358979323846 / 180.0 );
        /* Best-effort mapping into MIX 3D coordinates */
        pos.x = SDL_sinf( rad ) * d;
        pos.y = 0.0f;
        pos.z = -SDL_cosf( rad ) * d;
        MIX_SetTrack3DPosition( channels[canal], &pos );
        return ( 0 ) ;
    }
    return ( -1 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : set_distance
 *
 *  Set the "distance" of a channel. (distance) is an integer from 0 to 255
 *  that specifies the location of the sound in relation to the listener.
 *
 *  PARAMS:
 *
 *  channel
 *
 *  distance (0-255)
 *
 */

static int set_distance( int canal, int dist )
{
    MIX_Point3D pos;

    if ( !audio_initialized && sound_init() ) return ( -1 );
    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );

    if ( MIX_TrackPlaying( channels[canal] ) )
    {
        if ( dist < 0 ) dist = 0;
        if ( dist > 255 ) dist = 255;
        pos.x = 0.0f;
        pos.y = 0.0f;
        pos.z = ( float ) dist / 255.0f;
        MIX_SetTrack3DPosition( channels[canal], &pos );
        return ( 0 ) ;
    }

    return ( -1 ) ;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : reverse_stereo
 *
 *  Causes a channel to reverse its stereo.
 *
 *  PARAMS:
 *
 *  channel
 *  flip  0 normal  != reverse
 *
 */

static int reverse_stereo( int canal, int flip )
{
    int chmap[2];

    if ( !audio_initialized && sound_init() ) return ( -1 );
    if ( canal < 0 || canal >= num_channels || !channels[canal] ) return ( -1 );

    if ( MIX_TrackPlaying( channels[canal] ) )
    {
        if ( flip )
        {
            chmap[0] = 1;
            chmap[1] = 0;
        }
        else
        {
            chmap[0] = 0;
            chmap[1] = 1;
        }
        MIX_SetTrackOutputChannelMap( channels[canal], chmap, 2 );
        return ( 0 ) ;
    }

    return ( -1 ) ;
}

/* --------------------------------------------------------------------------- */
/* Sonido                                                                      */
/* --------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_load_song
 *
 *  Load a MOD from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE:
 *
 *      mod id
 *
 */

static int modsound_load_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    int var;
    const char * filename ;

    if ( !( filename = string_get( params[0] ) ) ) return ( 0 ) ;

    var = load_song( filename );
    string_discard( params[0] );

    return ( var );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_load_song2
 *
 *  Load a MOD from a file
 *
 *  PARAMS:
 *      file name
 *      pointer mod id
 *
 *  RETURN VALUE:
 *
 *
 */

static int modsound_bgload_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    bgload( load_song, params );
#else
    *(int *)(params[1]) = -1;
#endif
    return 0;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_play_song
 *
 *  Play a MOD
 *
 *  PARAMS:
 *      mod id;
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_play_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    if ( params[0] == -1 ) return -1;
    return( play_song( params[0], params[1] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_unload_song
 *
 *  Frees the resources from a MOD and unloads it
 *
 *  PARAMS:
 *      mod id;
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_unload_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    if ( params[0] == -1 ) return ( -1 );
    return( unload_song( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_unload_song2
 *
 *  Frees the resources from a MOD and unloads it
 *
 *  PARAMS:
 *      mod *id;
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_unload_song2( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    int *s = (int *)(params[0]), r;
    if ( !s || *s == -1 ) return ( -1 );
    r = unload_song( *s );
    *s = 0;
    return( r );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_stop_song
 *
 *  Stop the play of a mod
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_stop_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( stop_song() );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_pause_song
 *
 *  Pause the mod in curse, you can resume it after
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_pause_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( pause_song() );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_resume_song
 *
 *  Resume the mod, paused before
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_resume_song( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( resume_song() );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_is_playing_song
 *
 *  Check if there is any mod playing
 *
 *  PARAMS:
 *
 *  no params
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  TRUE OR FALSE if there is no error
 *
 */

static int modsound_is_playing_song( INSTANCE * my, intptr_t * params )
{
    return ( is_playing_song() );
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_set_song_volume
 *
 *  Set the volume for mod playing (0-128)
 *
 *  PARAMS:
 *
 *  int volume
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if there is no error
 *
 */

static int modsound_set_song_volume( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return ( set_song_volume( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_fade_music_in
 *
 *  Play a MOD/OGG fading in it
 *
 *  PARAMS:
 *      mod pointer
 *      number of loops (-1 infinite loops)
 *      ms  microsends of fadding
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *
 */

static int modsound_fade_music_in( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    if ( params[0] == -1 ) return -1;
    return ( fade_music_in( params[0], params[1], params[2] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_fade_music_off
 *
 *  Stop the play of a mod
 *
 *  PARAMS:
 *
 *  ms  microsends of fadding
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *
 */

static int modsound_fade_music_off( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return ( fade_music_off( params[0] ) );
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_load_wav
 *
 *  Load a WAV from a file
 *
 *  PARAMS:
 *      file name
 *
 *  RETURN VALUE:
 *
 *      wav id
 *
 */

static int modsound_load_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    int var;
    const char * filename ;

    if ( !( filename = string_get( params[0] ) ) ) return ( 0 ) ;

    var = load_wav( filename );
    string_discard( params[0] );

    return ( var );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_load_wav2
 *
 *  Load a WAV from a file
 *
 *  PARAMS:
 *      file name
 *      pointer wav id
 *
 *  RETURN VALUE:
 *
 *
 */

static int modsound_bgload_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    bgload( load_wav, params );
#else
    *(int *)(params[1]) = -1;
#endif
    return 0;
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_play_wav
 *
 *  Play a WAV
 *
 *  PARAMS:
 *      wav id;
 *      number of loops (-1 infinite loops)
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_play_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    if ( params[0] == -1 ) return -1;
    return( play_wav( params[0], params[1], -1 ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_play_wav_channel
 *
 *  Play a WAV
 *
 *  PARAMS:
 *      wav id;
 *      number of loops (-1 infinite loops)
 *      channel (-1 like modsound_play_wav)
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_play_wav_channel( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    if ( params[0] == -1 ) return -1;
    return( play_wav( params[0], params[1], params[2] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_unload_wav
 *
 *  Frees the resources from a wav, unloading it
 *
 *  PARAMS:
 *
 *  mod id
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_unload_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    if ( params[0] == -1 ) return -1;
    return( unload_wav( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_unload_wav2
 *
 *  Frees the resources from a wav, unloading it
 *
 *  PARAMS:
 *
 *  mod *id
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_unload_wav2( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    int *s = (int *)(params[0]), r;
    if ( !s || *s == -1 ) return ( -1 );
    r = unload_wav( *s );
    *s = 0;
    return( r );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_stop_wav
 *
 *  Stop a wav playing
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_stop_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( stop_wav( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_pause_wav
 *
 *  Pause a wav playing, you can resume it after
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_pause_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return ( pause_wav( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : resume_wav
 *
 *  Resume a wav playing, paused before
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if all goes ok
 *
 */

static int modsound_resume_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return ( resume_wav( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : is_playing_wav
 *
 *  Check a wav playing
 *
 *  PARAMS:
 *
 *  wav id
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  TRUE OR FALSE if there is no error
 *
 */


static int modsound_is_playing_wav( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return ( is_playing_wav( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_set_channel_volume
 *
 *  Set the volume for a wav playing (0-128)
 *
 *  PARAMS:
 *
 *  wav id
 *  int volume
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if there is no error
 *
 */

static int modsound_set_channel_volume( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( set_channel_volume( params[0], params[1] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_reserve_channels
 *
 *  Reserve the first channels (0 -> n-1) for the application, i.e. don't allocate
 *  them dynamically to the next sample if requested with a -1 value below.
 *
 *  PARAMS:
 *  number of channels to reserve.
 *
 *  RETURN VALUE:
 *  number of reserved channels.
 *  -1 if there is any error
 *
 */

static int modsound_reserve_channels( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return ( reserve_channels( params[0] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_set_wav_volume
 *
 *  Set the volume for a wav playing (0-128)
 *
 *  PARAMS:
 *
 *  wav id
 *  int volume
 *
 *  RETURN VALUE:
 *
 *  -1 if there is any error
 *  0 if there is no error
 *
 */

static int modsound_set_wav_volume( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( set_wav_volume( params[0], params[1] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_set_panning
 *
 *  Set the panning for a wav channel
 *
 *  PARAMS:
 *
 *  channel
 *  left volume (0-255)
 *  right volume (0-255)
 *
 */

static int modsound_set_panning( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( set_panning( params[0], params[1], params[2] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_set_position
 *
 *  Set the position of a channel. (angle) is an integer from 0 to 360
 *
 *  PARAMS:
 *
 *  channel
 *  angle (0-360)
 *  distance (0-255)
 *
 */

static int modsound_set_position( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( set_position( params[0], params[1], params[2] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_set_distance
 *
 *  Set the "distance" of a channel. (distance) is an integer from 0 to 255
 *  that specifies the location of the sound in relation to the listener.
 *
 *  PARAMS:
 *
 *  channel
 *
 *  distance (0-255)
 *
 */

static int modsound_set_distance( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( set_distance( params[0], params[1] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : modsound_reverse_stereo
 *
 *  Causes a channel to reverse its stereo.
 *
 *  PARAMS:
 *
 *  channel
 *
 *  flip 0 normal != reverse
 *
 */

static int modsound_reverse_stereo( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( reverse_stereo( params[0], params[1] ) );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */

static int modsound_set_music_position( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    float seconds;
    Sint64 frames;

    if ( !audio_initialized || !music_track ) return -1;
    seconds = *( float * ) &params[0];
    if ( seconds < 0.0f ) seconds = 0.0f;
    frames = MIX_TrackMSToFrames( music_track, ( Sint64 )( seconds * 1000.0f ) );
    if ( frames < 0 ) return -1;
    return MIX_SetTrackPlaybackPosition( music_track, frames ) ? 0 : -1;
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */

static int modsound_init( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    return( sound_init() );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */

static int modsound_close( INSTANCE * my, intptr_t * params )
{
#ifndef TARGET_DINGUX_A320
    sound_close();
    return( 0 );
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/* Funciones de inicializacion del modulo/plugin                               */

void  __bgdexport( mod_sound, module_initialize )()
{
#ifndef TARGET_DINGUX_A320
    if ( !SDL_WasInit( SDL_INIT_AUDIO ) ) SDL_InitSubSystem( SDL_INIT_AUDIO );
#endif
}

/* --------------------------------------------------------------------------- */

void __bgdexport( mod_sound, module_finalize )()
{
#ifndef TARGET_DINGUX_A320
    sound_close();
    if ( SDL_WasInit( SDL_INIT_AUDIO ) ) SDL_QuitSubSystem( SDL_INIT_AUDIO );
#endif
}

/* ----------------------------------------------------------------- */
/* exports                                                           */
/* ----------------------------------------------------------------- */

#include "mod_sound_exports.h"

/* ----------------------------------------------------------------- */
