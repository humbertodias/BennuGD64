/* SDL timer backend for Original Xbox / nxdk. */
#include "SDL_internal.h"

#ifdef SDL_TIMER_WINDOWS

#include <windows.h>
#include <winextras.h>

Uint64 SDL_GetPerformanceCounter(void)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (Uint64)counter.QuadPart;
}

Uint64 SDL_GetPerformanceFrequency(void)
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return (Uint64)frequency.QuadPart;
}

void SDL_SYS_DelayNS(Uint64 ns)
{
    Uint64 ms = ns / 1000000u;
    if (ms == 0 && ns > 0) {
        ms = 1;
    }
    if (ms > 0xffffffffu) {
        ms = 0xffffffffu;
    }
    Sleep((DWORD)ms);
}

#endif /* SDL_TIMER_WINDOWS */
