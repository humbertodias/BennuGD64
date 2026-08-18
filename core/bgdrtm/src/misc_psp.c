/*
 * PlayStation Portable runtime tweaks. Compiled only into the psp-mips build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_psp.h"

void bgdrtm_psp_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard; otherwise P1 is joy 0. */
    GLODWORD( OS_ID ) = OS_LINUX;
}
