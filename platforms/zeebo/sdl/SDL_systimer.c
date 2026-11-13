/*
 * SDL timer for the Zeebo ELF. Time is the emulator's instruction clock
 * (aee_GetTimeMS). A fake tick counter made every frame look instant, so
 * the limiter never noticed a late blit.
 */

#include "SDL_internal.h"

#define ZEEBO_MODULE_BASE 0x00010000u
#define ZEEBO_GETTIMEMS_SLOT 43

typedef Uint32 (*zeebo_ms_fn)(void);

static Uint32 zeebo_ms(void)
{
    void **table = *(void ***)(ZEEBO_MODULE_BASE - 4);
    zeebo_ms_fn fn;

    if (!table)
        return 0;
    fn = (zeebo_ms_fn)(uintptr_t)table[ZEEBO_GETTIMEMS_SLOT];
    if (!fn)
        return 0;
    return fn();
}

Uint64 SDL_GetPerformanceCounter(void)
{
    return (Uint64)zeebo_ms() * 1000u;
}

Uint64 SDL_GetPerformanceFrequency(void)
{
    return 1000000;
}

void SDL_SYS_DelayNS(Uint64 ns)
{
    Uint32 start;
    Uint32 want;
    Uint32 spins;

    /* One shot from SDL_Delay. Spin until the instruction clock covers the
     * request, so a 30 fps wait is real time for the emulator's limiter. */
    want = (Uint32)(ns / 1000000ull);
    if (want < 1)
        want = 1;
    if (want > 50)
        want = 50;
    start = zeebo_ms();
    spins = 0;
    while ((Uint32)(zeebo_ms() - start) < want) {
        if (++spins > 2000000u)
            break;
    }
}
