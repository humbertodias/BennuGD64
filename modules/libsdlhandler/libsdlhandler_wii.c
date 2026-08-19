/*
 * Wii Power/Reset poll after SDL_PumpEvents.
 * Compiled only into the wii-powerpc build.
 */

#include "libsdlhandler_wii.h"

void libsdlhandler_wii_after_pump( void )
{
    extern void bgdi_wii_handle_power( void );
    bgdi_wii_handle_power();
}
