/*
 * Target frame period for SET_FPS. On the web the interpreter is driven by
 * requestAnimationFrame and uses this period to decide when to run a FRAME.
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

int bgdrtm_rafs_per_frame( void )
{
    const double period = bgdrtm_frame_period_ms();
    int n = ( int )( period / ( 1000.0 / 60.0 ) + 0.5 );
    if ( n < 1 ) n = 1;
    if ( n > 4 ) n = 4;
    return n;
}

void bgdrtm_frame_throttle( void )
{
}
