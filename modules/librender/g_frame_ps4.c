/* Native PS4 frame timing; SDL timers require unsupported pthread state. */

#include <stdint.h>
#include <orbis/libkernel.h>

#include "g_frame_ps4.h"

int gr_frame_ps4_get_ticks_ms( void )
{
    return ( int )( sceKernelGetProcessTime() / 1000 );
}

void gr_frame_ps4_delay_ms( int delay )
{
    if ( delay > 0 )
        sceKernelUsleep( ( uint32_t ) delay * 1000u );
}
