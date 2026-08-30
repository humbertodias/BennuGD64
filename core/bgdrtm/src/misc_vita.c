/*
 * PlayStation Vita runtime tweaks. Compiled only into the vita-arm build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_vita.h"

void bgdrtm_vita_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard; otherwise P1 is joy 0. */
    GLODWORD( OS_ID ) = OS_LINUX;
}
