#include <windows.h>
#include "g_frame_xbox.h"

int gr_frame_xbox_get_ticks_ms( void )
{
    return ( int )GetTickCount();
}

void gr_frame_xbox_delay_ms( int delay )
{
    if ( delay > 0 ) Sleep( ( DWORD )delay );
}
