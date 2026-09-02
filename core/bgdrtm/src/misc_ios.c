/*
 * Apple iOS runtime tweaks. Compiled only into the ios-arm64 build.
 */

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_ios.h"

void bgdrtm_ios_entry( void )
{
    /* SoRR: os_id 0 (Windows) → P1 keyboard. Touch and MFi pads are
     * mapped onto the keyboard in libkey_ios.c. */
    GLODWORD( OS_ID ) = OS_WIN32;
}
