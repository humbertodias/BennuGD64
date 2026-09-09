/*
 * PlayStation 4 runtime tweaks. Compiled only into the ps4-x86_64 build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_ps4.h"

void bgdrtm_ps4_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard; otherwise P1 is joy 0.
     * PS4 pad is SDL joystick 0 when the SDL Orbis backend exposes it. */
    GLODWORD( OS_ID ) = OS_LINUX;
}
