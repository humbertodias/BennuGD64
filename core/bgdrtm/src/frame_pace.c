/*
 * Target frame period for SET_FPS. On the web the interpreter is driven by
 * requestAnimationFrame and uses elapsed time (not a 60 Hz RAF counter) so
 * 120 Hz displays do not run the game — and therefore key() — twice as fast.
 */

#include "bgdrtm.h"

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

#ifndef TARGET_EMSCRIPTEN
int bgdrtm_browser_frame_due( void )
{
    return 1;
}
#endif

void bgdrtm_frame_throttle( void )
{
}
