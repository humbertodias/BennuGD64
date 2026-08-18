/*
 * Browser joystick hotplug. Compiled only into the wasm build.
 */

#include <string.h>

#include "libjoy_internal.h"
#include "libjoy_emscripten.h"

static int libjoy_slot_for_id( SDL_JoystickID id )
{
    int i;
    if ( !id ) return -1;
    for ( i = 0; i < _max_joys; i++ )
        if ( _joystick_ids[ i ] == id ) return i;
    return -1;
}

static int libjoy_first_free_slot( void )
{
    int i;
    for ( i = 0; i < _max_joys; i++ )
        if ( !_joysticks[ i ] ) return i;
    return -1;
}

static int libjoy_attach( SDL_JoystickID id )
{
    int slot;
    SDL_Joystick * js;

    slot = libjoy_slot_for_id( id );
    if ( slot >= 0 && _joysticks[ slot ] )
        return slot;

    js = SDL_OpenJoystick( id );
    if ( !js )
        return -1;

    slot = ( slot >= 0 ) ? slot : libjoy_first_free_slot();
    if ( slot < 0 )
    {
        if ( _max_joys >= MAX_JOYS )
        {
            SDL_CloseJoystick( js );
            return -1;
        }
        slot = _max_joys++;
    }

    if ( _joysticks[ slot ] && _joysticks[ slot ] != js )
        SDL_CloseJoystick( _joysticks[ slot ] );
    _joystick_ids[ slot ] = id;
    _joysticks[ slot ] = js;
    libjoy_remember_name( slot );
    return slot;
}

static void libjoy_detach( SDL_JoystickID id )
{
    int slot = libjoy_slot_for_id( id );
    if ( slot < 0 ) return;
    if ( _joysticks[ slot ] ) SDL_CloseJoystick( _joysticks[ slot ] );
    _joysticks[ slot ] = NULL;
    _joystick_ids[ slot ] = 0;
    _joystick_names[ slot ][ 0 ] = 0;
    while ( _max_joys > 0 && !_joysticks[ _max_joys - 1 ] )
        _max_joys--;
}

void libjoy_emscripten_compact_slots( void )
{
    int i, w = 0;

    for ( i = 0; i < _max_joys; i++ )
    {
        if ( !_joysticks[ i ] ) continue;
        if ( w != i )
        {
            _joysticks[ w ] = _joysticks[ i ];
            _joystick_ids[ w ] = _joystick_ids[ i ];
            memcpy( _joystick_names[ w ], _joystick_names[ i ], sizeof( _joystick_names[ w ] ) );
        }
        w++;
    }
    for ( i = w; i < _max_joys; i++ )
    {
        _joysticks[ i ] = NULL;
        _joystick_ids[ i ] = 0;
        _joystick_names[ i ][ 0 ] = 0;
    }
    _max_joys = w;
}

void libjoy_emscripten_refresh( void )
{
    SDL_Event e;
    int count = 0, i;
    SDL_JoystickID * ids;

    SDL_UpdateJoysticks();

    while ( SDL_PeepEvents( &e, 1, SDL_GETEVENT, SDL_EVENT_JOYSTICK_ADDED, SDL_EVENT_JOYSTICK_REMOVED ) > 0 )
    {
        if ( e.type == SDL_EVENT_JOYSTICK_ADDED )
            libjoy_attach( e.jdevice.which );
        else if ( e.type == SDL_EVENT_JOYSTICK_REMOVED )
            libjoy_detach( e.jdevice.which );
    }

    /* Gamepad API can expose a pad without an ADDED event if the grant
     * happened before SDL_InitSubSystem(JOYSTICK). */
    ids = SDL_GetJoysticks( &count );
    if ( !ids ) return;
    for ( i = 0; i < count; i++ )
        if ( libjoy_slot_for_id( ids[ i ] ) < 0 )
            libjoy_attach( ids[ i ] );
    SDL_free( ids );
}
