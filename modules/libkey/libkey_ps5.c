/*
 * PlayStation 5 keyboard state. Compiled only into the ps5-x86_64 build.
 *
 * The DualSense is already a joystick. SorR keeps reading the keyboard
 * after it enables that joystick, so mirroring the pad onto the arrow
 * keys made every menu press count twice. Held directions were worse:
 * the menu repeat lock opens whenever scan_code is 0, and a poked key
 * state never produces a key event, so the cursor advanced every frame.
 * A real keyboard still comes through SDL.
 */

#include <string.h>

#include <SDL3/SDL.h>

#include "libkey_ps5.h"

extern const bool * keystate;

static bool ps5_keystate[ SDL_SCANCODE_COUNT ];

void libkey_ps5_after_init( SDL_Window * window )
{
    ( void ) window;
    libkey_ps5_after_events();
}

void libkey_ps5_after_events( void )
{
    const bool * sdl;
    int n = 0;
    int copy;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( ps5_keystate, 0, sizeof( ps5_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( ps5_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    keystate = ps5_keystate;
}
