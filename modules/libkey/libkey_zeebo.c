/*
 * Same pad, as keys. SoRR on an OS it does not know reads the keyboard,
 * and the Zeebo port reports OS_ZEEBO. The map is the Xbox one's, which
 * is SoRR's default: arrows, C/Ctrl, V/Alt, X/Space, Enter.
 */

#include <string.h>

#include <SDL3/SDL.h>

#include "libjoy_zeebo.h"
#include "libkey_zeebo.h"

extern const bool * keystate;

static bool zeebo_keys[SDL_SCANCODE_COUNT];

static void zeebo_set( SDL_Scancode code )
{
    if ( code > 0 && code < SDL_SCANCODE_COUNT ) zeebo_keys[code] = true;
}

void libkey_zeebo_after_events( void )
{
    memset( zeebo_keys, 0, sizeof( zeebo_keys ) );
    libjoy_zeebo_pump();

    if ( libjoy_zeebo_get_hat( 0, 0 ) & SDL_HAT_UP ||
         libjoy_zeebo_get_position( 0, 1 ) < -16384 )
        zeebo_set( SDL_SCANCODE_UP );
    if ( libjoy_zeebo_get_hat( 0, 0 ) & SDL_HAT_DOWN ||
         libjoy_zeebo_get_position( 0, 1 ) > 16384 )
        zeebo_set( SDL_SCANCODE_DOWN );
    if ( libjoy_zeebo_get_hat( 0, 0 ) & SDL_HAT_LEFT ||
         libjoy_zeebo_get_position( 0, 0 ) < -16384 )
        zeebo_set( SDL_SCANCODE_LEFT );
    if ( libjoy_zeebo_get_hat( 0, 0 ) & SDL_HAT_RIGHT ||
         libjoy_zeebo_get_position( 0, 0 ) > 16384 )
        zeebo_set( SDL_SCANCODE_RIGHT );

    if ( libjoy_zeebo_get_button( 0, 0 ) )
    {
        zeebo_set( SDL_SCANCODE_A );
        zeebo_set( SDL_SCANCODE_C );
        zeebo_set( SDL_SCANCODE_LCTRL );
    }
    if ( libjoy_zeebo_get_button( 0, 1 ) )
    {
        zeebo_set( SDL_SCANCODE_D );
        zeebo_set( SDL_SCANCODE_V );
        zeebo_set( SDL_SCANCODE_LALT );
    }
    if ( libjoy_zeebo_get_button( 0, 2 ) )
    {
        zeebo_set( SDL_SCANCODE_S );
        zeebo_set( SDL_SCANCODE_X );
        zeebo_set( SDL_SCANCODE_SPACE );
    }
    if ( libjoy_zeebo_get_button( 0, 3 ) || libjoy_zeebo_get_button( 0, 5 ) )
        zeebo_set( SDL_SCANCODE_B );
    if ( libjoy_zeebo_get_button( 0, 8 ) || libjoy_zeebo_get_button( 0, 9 ) )
        zeebo_set( SDL_SCANCODE_RETURN );

    keystate = zeebo_keys;
}

void libkey_zeebo_after_init( SDL_Window * window )
{
    ( void )window;
    libkey_zeebo_after_events();
}
