/*
 * PlayStation 2 event pump. Compiled only into the ps2-mips build.
 *
 * SDL_PumpEvents on PCSX2 HostFS can stall the EE after the first FRAME.
 * Splash timers still advance; DualShock mapping comes later.
 */

#include "libsdlhandler_ps2.h"

void libsdlhandler_ps2_pump( void )
{
}
