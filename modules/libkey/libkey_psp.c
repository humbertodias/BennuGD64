/*
 * PlayStation Portable keyboard init. Compiled only into the psp-mips build.
 */

#include "libkey_psp.h"

void libkey_psp_after_init( SDL_Window * window )
{
    ( void ) window;
    /* Do not call SDL_StartTextInput: it opens the system OSK at startup. */
}
