/* Native PS5 frame timing. */

#include "g_frame_ps5.h"
#include "ps5_platform.h"

int gr_frame_ps5_get_ticks_ms( void )
{
    return ps5_platform_get_ticks_ms();
}

void gr_frame_ps5_delay_ms( int delay )
{
    ps5_platform_delay_ms( delay );
}
