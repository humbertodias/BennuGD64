/*
 * Browser keyboard: throttle SDL key repeat. Compiled only into the wasm build.
 */

#include "libkey_emscripten.h"

#define BENNU_WASM_REPEAT_DELAY_MS     280
#define BENNU_WASM_REPEAT_INTERVAL_MS   70

static Uint64 wasm_key_down_ms[ SDL_SCANCODE_COUNT ];
static Uint64 wasm_key_repeat_ms[ SDL_SCANCODE_COUNT ];

void libkey_emscripten_after_init( SDL_Window * window )
{
    ( void ) window;
    /* A hidden text field on the web steals focus after Enter (menus) and
     * the next screen (character select) gets no keys. ascii comes from
     * KEYDOWN, so text input is not needed. */
}

int libkey_emscripten_filter_keydown( const SDL_Event * e )
{
    Uint64 now;
    SDL_Scancode sc;

    if ( !e || e->key.scancode >= SDL_SCANCODE_COUNT )
        return 1;

    now = SDL_GetTicks();
    sc = e->key.scancode;
    if ( e->key.repeat )
    {
        if ( now - wasm_key_down_ms[ sc ] < BENNU_WASM_REPEAT_DELAY_MS )
            return 0;
        if ( now - wasm_key_repeat_ms[ sc ] < BENNU_WASM_REPEAT_INTERVAL_MS )
            return 0;
        wasm_key_repeat_ms[ sc ] = now;
    }
    else
    {
        wasm_key_down_ms[ sc ] = now;
        wasm_key_repeat_ms[ sc ] = now;
    }
    return 1;
}
