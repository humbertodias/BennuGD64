/*
 * PlayStation Vita event pump. Compiled only into the vita-arm build.
 *
 * SoRR-vita stopped flushing the SDL queue here: PeepEvents(GETEVENT) was
 * dropping joystick/key events other modules still needed. Pump the window
 * (GXM present) and refresh pads; do not drain the queue.
 */

#include <SDL3/SDL.h>

#include "libsdlhandler_vita.h"

void libsdlhandler_vita_pump( void )
{
    SDL_PumpEvents();
    SDL_UpdateJoysticks();
}
