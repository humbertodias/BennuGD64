/*
 * PlayStation 3 event pump. Compiled only into the ps3-ppu build.
 *
 * Pump the window and refresh pads; do not drain the SDL queue (other
 * modules still need those events).
 */

#include <SDL3/SDL.h>

#include "libsdlhandler_ps3.h"

void libsdlhandler_ps3_pump( void )
{
    SDL_PumpEvents();
    SDL_UpdateJoysticks();
}
