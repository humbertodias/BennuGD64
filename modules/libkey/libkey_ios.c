/*
 * Apple iOS Game Controller + touch → keyboard.
 * Compiled only into the ios-arm64 build.
 *
 *   D-pad / left-side stick   → arrows
 *   A / south / right-lower   → Space
 *   Start / right-upper       → Enter
 *   B / east                  → Escape
 * Phantom SDL Escape is cleared (hello.prg would exit with no key pressed).
 * The system Home / Guide button is not mapped (it must stay with iOS).
 *
 * OS_WIN32 in misc_ios.c makes SoRR use this keystate as P1.
 */

#include <string.h>

#include <SDL3/SDL.h>

#include "libkey_ios.h"

extern const bool * keystate;

static bool ios_keystate[ SDL_SCANCODE_COUNT ];

static SDL_Gamepad * ios_open_pad( void )
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

static void ios_apply_pad( bool * keys )
{
    SDL_Gamepad * pad = ios_open_pad();

    if ( !pad )
        return;

    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_UP ) )
        keys[ SDL_SCANCODE_UP ] = true;
    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN ) )
        keys[ SDL_SCANCODE_DOWN ] = true;
    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT ) )
        keys[ SDL_SCANCODE_LEFT ] = true;
    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT ) )
        keys[ SDL_SCANCODE_RIGHT ] = true;
    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_SOUTH ) )
        keys[ SDL_SCANCODE_SPACE ] = true;
    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_START ) )
        keys[ SDL_SCANCODE_RETURN ] = true;
    if ( SDL_GetGamepadButton( pad, SDL_GAMEPAD_BUTTON_EAST ) )
        keys[ SDL_SCANCODE_ESCAPE ] = true;
}

static void ios_apply_stick( bool * keys, float x, float y )
{
    const float dead = 0.22f;

    if ( y < -dead )
        keys[ SDL_SCANCODE_UP ] = true;
    if ( y > dead )
        keys[ SDL_SCANCODE_DOWN ] = true;
    if ( x < -dead )
        keys[ SDL_SCANCODE_LEFT ] = true;
    if ( x > dead )
        keys[ SDL_SCANCODE_RIGHT ] = true;
}

static void ios_apply_touches( bool * keys )
{
    SDL_TouchID * devices;
    int ndev = 0, d, f, nf;
    SDL_Finger ** fingers;

    devices = SDL_GetTouchDevices( &ndev );
    if ( !devices )
        return;

    for ( d = 0 ; d < ndev ; d++ )
    {
        fingers = SDL_GetTouchFingers( devices[ d ], &nf );
        if ( !fingers )
            continue;
        for ( f = 0 ; f < nf ; f++ )
        {
            float x, y;

            if ( !fingers[ f ] )
                continue;
            x = fingers[ f ]->x;
            y = fingers[ f ]->y;
            if ( x < 0.42f )
                ios_apply_stick( keys, ( x - 0.18f ) / 0.18f, ( y - 0.55f ) / 0.28f );
            else if ( x > 0.62f && y > 0.52f )
                keys[ SDL_SCANCODE_SPACE ] = true;
            else if ( x > 0.62f && y < 0.48f )
                keys[ SDL_SCANCODE_RETURN ] = true;
        }
        SDL_free( fingers );
    }
    SDL_free( devices );
}

void libkey_ios_after_init( SDL_Window * window )
{
    ( void ) window;
    if ( !SDL_WasInit( SDL_INIT_GAMEPAD ) )
        SDL_InitSubSystem( SDL_INIT_GAMEPAD );
    libkey_ios_after_events();
}

void libkey_ios_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( ios_keystate, 0, sizeof( ios_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( ios_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    ios_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    ios_apply_pad( ios_keystate );
    ios_apply_touches( ios_keystate );

    keystate = ios_keystate;
}
