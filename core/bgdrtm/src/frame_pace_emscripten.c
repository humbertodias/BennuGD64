/*
 * Browser frame pacing. Compiled only into the wasm build.
 */

#include <emscripten.h>
#include <SDL3/SDL.h>

#include "bgdrtm.h"

int bgdrtm_browser_frame_due( void )
{
    static double last_ms = -1.0;
    static double acc_ms = 0.0;
    const double now = emscripten_get_now();
    const double period = bgdrtm_frame_period_ms();

    if ( last_ms < 0.0 )
    {
        last_ms = now;
        return 1;
    }

    acc_ms += now - last_ms;
    last_ms = now;

    if ( acc_ms < period )
    {
        /* Keep KEYUP flowing while we skip rAFs, or Enter stays stuck. */
        SDL_PumpEvents();
        return 0;
    }

    acc_ms -= period;
    /* Drop only a large backlog (backgrounded tab). Small leftover keeps SET_FPS. */
    if ( acc_ms > period )
        acc_ms = 0.0;
    return 1;
}
