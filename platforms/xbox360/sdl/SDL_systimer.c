/*
 * SDL timer backend for the open libXenon Xbox 360 runtime.
 * Kept outside SDL so the platform dependency stays owned by this port.
 */

#include "SDL_internal.h"

#include <ppc/timebase.h>
#include <time/time.h>

Uint64 SDL_GetPerformanceCounter(void)
{
    return (Uint64)mftb();
}

Uint64 SDL_GetPerformanceFrequency(void)
{
    return (Uint64)PPC_TIMEBASE_FREQ;
}

void SDL_SYS_DelayNS(Uint64 ns)
{
    Uint64 us = (ns + 999u) / 1000u;

    while (us > 1000000u)
    {
        udelay(1000000);
        us -= 1000000u;
    }
    if (us)
        udelay((int)us);
}
