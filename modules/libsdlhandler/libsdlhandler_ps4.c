/*
 * PlayStation 4 event pump. Compiled only into the ps4-x86_64 build.
 */

#include <SDL3/SDL.h>

#include "libsdlhandler_ps4.h"

void libsdlhandler_ps4_pump( void )
{
    SDL_PumpEvents();
    SDL_UpdateJoysticks();
}
