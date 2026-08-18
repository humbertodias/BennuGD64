/*
 * Emscripten / browser video: keep the canvas, reset keys after set_mode.
 */

#include "libvideo.h"
#include "g_video_emscripten.h"

void gr_video_emscripten_module_initialize( void )
{
    SDL_SetHint( SDL_HINT_EMSCRIPTEN_ASYNCIFY, "1" );
    /* Listen on the window, not the canvas: after Enter, page chrome can
     * steal canvas focus and in-game keys would stop. */
    SDL_SetHint( "SDL_EMSCRIPTEN_KEYBOARD_ELEMENT", "#window" );
}

int gr_video_emscripten_should_recreate_window( void )
{
    /* Destroy+CreateWindow stacks Emscripten key listeners, so every
     * physical keypress becomes two SDL_EVENT_KEY_DOWN. Resize in place. */
    return 0;
}

void gr_video_emscripten_after_set_mode( void )
{
    /* set_mode on character select can drop the menu Enter KEYUP; SoRR then
     * waits forever for that key to be released. */
    SDL_ResetKeyboard();
}
