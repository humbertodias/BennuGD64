/*
 * Nintendo Wii frame pacing. SDL_GetTicks / SDL_Delay only advance at
 * vsync/present in this SDL3-libogc2 build, and gr_wait_frame sleeps
 * before present — so Delay never returns. Use the CPU timebase instead.
 * Compiled only into the wii-powerpc build.
 */

#include <ogc/lwp.h>
#include <ogc/lwp_watchdog.h>

#include "g_frame_wii.h"

int gr_frame_wii_get_ticks_ms( void )
{
    return ( int ) ticks_to_millisecs( gettime() );
}

void gr_frame_wii_delay_ms( int delay )
{
    u64 target;

    if ( delay <= 0 )
        return;
    target = gettime() + millisecs_to_ticks( (u32) delay );
    while ( gettime() < target )
        LWP_YieldThread();
}
