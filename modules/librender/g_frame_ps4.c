/* Native PS4 frame timing; SDL timers require unsupported pthread state. */

#include "g_frame_ps4.h"
#include "ps4_platform.h"

int gr_frame_ps4_get_ticks_ms( void )
{
    return ps4_platform_get_ticks_ms();
}

void gr_frame_ps4_delay_ms( int delay )
{
    ps4_platform_delay_ms( delay );
}
