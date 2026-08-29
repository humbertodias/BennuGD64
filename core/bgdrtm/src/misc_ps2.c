/*
 * PlayStation 2 runtime tweaks. Compiled only into the ps2-mips build.
 */

#include <SDL3/SDL.h>

#include "bgdrtm.h"
#include "offsets.h"
#include "misc_ps2.h"

int bgdrtm_ps2_video_ready( void )
{
    return SDL_WasInit( SDL_INIT_VIDEO ) ? 1 : 0;
}

void bgdrtm_ps2_entry( void )
{
    /* SoRR: os_id 0 (Windows) is keyboard P1. DualShock is mapped to KEY(). */
    GLODWORD( OS_ID ) = OS_WIN32;
}
