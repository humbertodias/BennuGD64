/*
 * Apple tvOS runtime tweaks. Compiled only into the tvos-arm64 build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_tvos.h"

void bgdrtm_tvos_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard; otherwise P1 is joy 0.
     * The Siri Remote / Game Controller is SDL gamepad 0. */
    GLODWORD( OS_ID ) = OS_LINUX;
}
