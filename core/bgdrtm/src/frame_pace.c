/*
 * Target frame period for SET_FPS. On the web the interpreter is driven by
 * requestAnimationFrame and uses elapsed time (not a 60 Hz RAF counter) so
 * 120 Hz displays do not run the game — and therefore key() — twice as fast.
 */

#include "bgdrtm.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL3/SDL.h>
#endif

static float pace_ms = 40.0f;

void bgdrtm_set_frame_pace( int fps )
{
    pace_ms = fps > 0 ? 1000.0f / ( float ) fps : 0.0f;
}

double bgdrtm_frame_period_ms( void )
{
    /* SET_FPS(0) is "unlimited" on desktop (compositor vsync ~60 Hz). */
    return pace_ms > 0.0f ? ( double ) pace_ms : ( 1000.0 / 60.0 );
}

int bgdrtm_browser_frame_due( void )
{
#ifdef __EMSCRIPTEN__
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
#else
    return 1;
#endif
}

void bgdrtm_frame_throttle( void )
{
}
