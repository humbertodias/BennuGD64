/*
 * Apple tvOS Siri Remote / Game Controller → keyboard.
 * Compiled only into the tvos-arm64 build.
 *
 *   D-pad / touch     → arrows
 *   Select / A / click → Space
 *   Play/Pause / Start → Enter
 *   Menu / B / Back    → Escape
 * Phantom SDL Escape is cleared (hello.prg would exit with no key pressed).
 *
 * OS_LINUX in misc_tvos.c makes SoRR use joy 0 as P1.
 */

#include <string.h>

#include <SDL3/SDL.h>

#include "libkey_tvos.h"

extern const bool * keystate;

static bool tvos_keystate[ SDL_SCANCODE_COUNT ];

static SDL_Gamepad * tvos_open_pad( void )
{
    SDL_JoystickID * ids;
    int n = 0;
    SDL_Gamepad * pad = NULL;

    ids = SDL_GetGamepads( &n );
    if ( !ids || n < 1 )
        return NULL;
    pad = SDL_GetGamepadFromID( ids[0] );
    if ( !pad )
        pad = SDL_OpenGamepad( ids[0] );
    SDL_free( ids );
    return pad;
}

void libkey_tvos_after_init( SDL_Window * window )
{
    ( void ) window;
    if ( !SDL_WasInit( SDL_INIT_GAMEPAD ) )
        SDL_InitSubSystem( SDL_INIT_GAMEPAD );
    libkey_tvos_after_events();
}

void libkey_tvos_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;
    SDL_Gamepad * pad;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( tvos_keystate, 0, sizeof( tvos_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( tvos_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    tvos_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    pad = tvos_open_pad();
    if ( pad )
    {
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_UP ) )
            tvos_keystate[ SDL_SCANCODE_UP ] = true;
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN ) )
            tvos_keystate[ SDL_SCANCODE_DOWN ] = true;
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT ) )
            tvos_keystate[ SDL_SCANCODE_LEFT ] = true;
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT ) )
            tvos_keystate[ SDL_SCANCODE_RIGHT ] = true;
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_SOUTH ) )
            tvos_keystate[ SDL_SCANCODE_SPACE ] = true;
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_START ) )
            tvos_keystate[ SDL_SCANCODE_RETURN ] = true;
        if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_EAST ) ||
             SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_BACK ) ||
             SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_GUIDE ) )
            tvos_keystate[ SDL_SCANCODE_ESCAPE ] = true;
    }

    keystate = tvos_keystate;
}
