/*
 * PlayStation 5 runtime tweaks. Compiled only into the ps5-x86_64 build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_ps5.h"

void bgdrtm_ps5_entry( void )
{
    /* SoRR: os_id 0 (Windows) means P1 keyboard; any other id uses joy 0.
     * The DualSense is exposed as joystick 0 by libjoy_ps5. */
    GLODWORD( OS_ID ) = OS_LINUX;
}
