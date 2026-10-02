/*
 * SDL timer for the Zeebo ELF. BREW has no clock wired into newlib,
 * so this counts a software tick and burns a short loop in SDL_Delay.
 */

#include "SDL_internal.h"

static Uint64 zeebo_ticks;

Uint64 SDL_GetPerformanceCounter(void)
{
    return ++zeebo_ticks;
}

Uint64 SDL_GetPerformanceFrequency(void)
{
    return 1000000;
}

void SDL_SYS_DelayNS(Uint64 ns)
{
    volatile Uint64 left = ns / 1000u;

    if (left < 1)
        left = 1;
    while (left--)
        zeebo_ticks++;
}
