#include <string.h>

#include <SDL3/SDL.h>
#include "libjoy_xbox.h"
#include "libkey_xbox.h"

extern const bool * keystate;

static bool xbox_keys[SDL_SCANCODE_COUNT];

static void xbox_set( SDL_Scancode code )
{
    if ( code > 0 && code < SDL_SCANCODE_COUNT ) xbox_keys[code] = true;
}

void libkey_xbox_after_events( void )
{
    memset( xbox_keys, 0, sizeof( xbox_keys ) );
    libjoy_xbox_pump();

    if ( libjoy_xbox_get_hat( 0, 0 ) & SDL_HAT_UP ||
         libjoy_xbox_get_position( 0, 1 ) < -16384 )
        xbox_set( SDL_SCANCODE_UP );
    if ( libjoy_xbox_get_hat( 0, 0 ) & SDL_HAT_DOWN ||
         libjoy_xbox_get_position( 0, 1 ) > 16384 )
        xbox_set( SDL_SCANCODE_DOWN );
    if ( libjoy_xbox_get_hat( 0, 0 ) & SDL_HAT_LEFT ||
         libjoy_xbox_get_position( 0, 0 ) < -16384 )
        xbox_set( SDL_SCANCODE_LEFT );
    if ( libjoy_xbox_get_hat( 0, 0 ) & SDL_HAT_RIGHT ||
         libjoy_xbox_get_position( 0, 0 ) > 16384 )
        xbox_set( SDL_SCANCODE_RIGHT );

    /* Match xSorR / Streets of Rage Remake defaults. */
    if ( libjoy_xbox_get_button( 0, 0 ) )
    {
        xbox_set( SDL_SCANCODE_A );
        xbox_set( SDL_SCANCODE_C );
        xbox_set( SDL_SCANCODE_LCTRL );
    }
    if ( libjoy_xbox_get_button( 0, 1 ) )
    {
        xbox_set( SDL_SCANCODE_D );
        xbox_set( SDL_SCANCODE_V );
        xbox_set( SDL_SCANCODE_LALT );
    }
    if ( libjoy_xbox_get_button( 0, 2 ) )
    {
        xbox_set( SDL_SCANCODE_S );
        xbox_set( SDL_SCANCODE_X );
        xbox_set( SDL_SCANCODE_SPACE );
    }
    if ( libjoy_xbox_get_button( 0, 3 ) )
        xbox_set( SDL_SCANCODE_B );
    if ( libjoy_xbox_get_button( 0, 5 ) )
        xbox_set( SDL_SCANCODE_B );
    if ( libjoy_xbox_get_button( 0, 8 ) )
        xbox_set( SDL_SCANCODE_RETURN );
    if ( libjoy_xbox_get_button( 0, 9 ) )
        xbox_set( SDL_SCANCODE_ESCAPE );

    keystate = xbox_keys;
}

void libkey_xbox_after_init( SDL_Window * window )
{
    ( void )window;
    libkey_xbox_after_events();
}
