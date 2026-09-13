#include <ppc/timebase.h>
#include <time/time.h>

#include "g_frame_xbox360.h"

int gr_frame_xbox360_get_ticks_ms( void )
{
    return ( int )( mftb() / ( PPC_TIMEBASE_FREQ / 1000u ) );
}

void gr_frame_xbox360_delay_ms( int delay )
{
    if ( delay > 0 ) mdelay( delay );
}
