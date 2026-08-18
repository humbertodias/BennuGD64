/*
 * Browser event pump. Compiled only into the wasm build.
 */

#include <SDL3/SDL.h>
#include "libsdlhandler_emscripten.h"

void libsdlhandler_emscripten_pump( void )
{
    /* Do not drain the queue first. Emscripten pushes KEYUP into SDL from
     * the browser callback; discarding it left keys stuck down so games
     * that wait for Enter to be released (character select) froze. */
    SDL_PumpEvents();
}
