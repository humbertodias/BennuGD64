/*
 * Map Wii pads onto the keyboard SoRR (PC port) waits on at BOMBER_LOGO:
 * Enter / Space / arrows. Compiled only into the wii-powerpc build.
 */

#include <string.h>

#include <ogc/pad.h>
#include <wiiuse/wpad.h>

#include <SDL3/SDL.h>

#include "libkey_wii.h"

extern const bool * keystate;

static bool wii_keystate[ SDL_SCANCODE_COUNT ];

static void set_sc( SDL_Scancode sc )
{
    if ( sc < SDL_SCANCODE_COUNT )
        wii_keystate[ sc ] = true;
}

void libkey_wii_after_events( void )
{
    const bool * sdl;
    int n = 0, copy, i;
    u32 w;
    u16 g;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( wii_keystate, 0, sizeof( wii_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( wii_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    WPAD_ScanPads();
    PAD_ScanPads();

    for ( i = 0; i < 4; i++ )
    {
        w = WPAD_ButtonsHeld( i );
        g = PAD_ButtonsHeld( i );

        if ( ( w & ( WPAD_BUTTON_A | WPAD_BUTTON_PLUS | WPAD_BUTTON_2 |
                     WPAD_CLASSIC_BUTTON_A | WPAD_CLASSIC_BUTTON_PLUS ) ) ||
             ( g & ( PAD_BUTTON_A | PAD_BUTTON_START ) ) )
        {
            set_sc( SDL_SCANCODE_RETURN );
            set_sc( SDL_SCANCODE_SPACE );
        }

        if ( ( w & ( WPAD_BUTTON_B | WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_B ) ) ||
             ( g & PAD_BUTTON_B ) )
            set_sc( SDL_SCANCODE_ESCAPE );

        if ( ( w & ( WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP ) ) || ( g & PAD_BUTTON_UP ) )
            set_sc( SDL_SCANCODE_UP );
        if ( ( w & ( WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN ) ) || ( g & PAD_BUTTON_DOWN ) )
            set_sc( SDL_SCANCODE_DOWN );
        if ( ( w & ( WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT ) ) || ( g & PAD_BUTTON_LEFT ) )
            set_sc( SDL_SCANCODE_LEFT );
        if ( ( w & ( WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT ) ) || ( g & PAD_BUTTON_RIGHT ) )
            set_sc( SDL_SCANCODE_RIGHT );
    }

    keystate = wii_keystate;
}
