/*
 * PlayStation Vita pad → keyboard. Compiled only into the vita-arm build.
 *
 * hello.prg exits on key(_esc). SDL's Vita keyboard map can report Escape
 * with no key pressed; that looks like the bubble opening and closing.
 * SELECT is Escape (same as PS2); START is Enter.
 */

#include <string.h>

#include <psp2/ctrl.h>
#include <SDL3/SDL.h>

#include "libkey_vita.h"

extern const bool * keystate;

static bool vita_keystate[ SDL_SCANCODE_COUNT ];

void libkey_vita_after_init( SDL_Window * window )
{
    ( void ) window;
    sceCtrlSetSamplingMode( SCE_CTRL_MODE_ANALOG );
    libkey_vita_after_events();
}

void libkey_vita_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;
    SceCtrlData pad;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( vita_keystate, 0, sizeof( vita_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( vita_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    vita_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    memset( &pad, 0, sizeof( pad ) );
    sceCtrlPeekBufferPositive( 0, &pad, 1 );

    if ( pad.buttons & SCE_CTRL_UP )
        vita_keystate[ SDL_SCANCODE_UP ] = true;
    if ( pad.buttons & SCE_CTRL_DOWN )
        vita_keystate[ SDL_SCANCODE_DOWN ] = true;
    if ( pad.buttons & SCE_CTRL_LEFT )
        vita_keystate[ SDL_SCANCODE_LEFT ] = true;
    if ( pad.buttons & SCE_CTRL_RIGHT )
        vita_keystate[ SDL_SCANCODE_RIGHT ] = true;
    if ( pad.buttons & SCE_CTRL_CROSS )
        vita_keystate[ SDL_SCANCODE_SPACE ] = true;
    if ( pad.buttons & SCE_CTRL_START )
        vita_keystate[ SDL_SCANCODE_RETURN ] = true;
    if ( pad.buttons & SCE_CTRL_SELECT )
        vita_keystate[ SDL_SCANCODE_ESCAPE ] = true;

    keystate = vita_keystate;
}
