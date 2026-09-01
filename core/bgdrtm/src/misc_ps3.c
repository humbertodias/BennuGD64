/*
 * PlayStation 3 runtime tweaks. Compiled only into the ps3-ppu build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_ps3.h"

void bgdrtm_ps3_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard; otherwise P1 is joy 0.
     * PS3 pad is SDL joystick 0 (D-pad as hat, analog, face buttons). */
    GLODWORD( OS_ID ) = OS_LINUX;
}
