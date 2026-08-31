/*
 * PlayStation 3 DualShock → keyboard. Compiled only into the ps3-ppu build.
 *
 * Keyboard (key() / SCANCODE), via ioPad on top of SDL_GetKeyboardState:
 *   D-pad     → arrows
 *   Cross     → Space
 *   START     → Enter
 *   SELECT    → Escape
 * Phantom SDL Escape is cleared (hello.prg would exit with no key pressed).
 *
 * Joystick (JOY_*): SDL's PS3 pad as joy 0. OS_LINUX in misc_ps3.c makes
 * SoRR use that pad as P1 instead of the keyboard.
 *
 * The event pump (libsdlhandler_ps3) PumpEvents + UpdateJoysticks and
 * does not drain the SDL queue.
 */

#include <string.h>

#include <io/pad.h>
#include <SDL3/SDL.h>

#include "libkey_ps3.h"

extern const bool * keystate;

static bool ps3_keystate[ SDL_SCANCODE_COUNT ];
static padData ps3_last_pad;
static int ps3_have_pad;

void libkey_ps3_after_init( SDL_Window * window )
{
    ( void ) window;
    ioPadInit( 7 );
    memset( &ps3_last_pad, 0, sizeof( ps3_last_pad ) );
    ps3_have_pad = 0;
    libkey_ps3_after_events();
}

void libkey_ps3_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;
    padInfo info;
    padData pad;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( ps3_keystate, 0, sizeof( ps3_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( ps3_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    ps3_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    memset( &info, 0, sizeof( info ) );
    memset( &pad, 0, sizeof( pad ) );
    ioPadGetInfo( &info );
    if ( info.status[0] )
    {
        ioPadGetData( 0, &pad );
        /* ioPadGetData only fills when the state changes; keep the last held. */
        if ( pad.len )
        {
            ps3_last_pad = pad;
            ps3_have_pad = 1;
        }
        else if ( ps3_have_pad )
            pad = ps3_last_pad;
    }

    if ( pad.BTN_UP )
        ps3_keystate[ SDL_SCANCODE_UP ] = true;
    if ( pad.BTN_DOWN )
        ps3_keystate[ SDL_SCANCODE_DOWN ] = true;
    if ( pad.BTN_LEFT )
        ps3_keystate[ SDL_SCANCODE_LEFT ] = true;
    if ( pad.BTN_RIGHT )
        ps3_keystate[ SDL_SCANCODE_RIGHT ] = true;
    if ( pad.BTN_CROSS )
        ps3_keystate[ SDL_SCANCODE_SPACE ] = true;
    if ( pad.BTN_START )
        ps3_keystate[ SDL_SCANCODE_RETURN ] = true;
    if ( pad.BTN_SELECT )
        ps3_keystate[ SDL_SCANCODE_ESCAPE ] = true;

    keystate = ps3_keystate;
}
