/* SDL power stub for nxdk. */
#include "SDL_internal.h"

#ifdef SDL_POWER_WINDOWS

bool SDL_GetPowerInfo_Windows(SDL_PowerState *state, int *seconds, int *percent)
{
    if (state) {
        *state = SDL_POWERSTATE_NO_BATTERY;
    }
    if (seconds) {
        *seconds = -1;
    }
    if (percent) {
        *percent = -1;
    }
    return true;
}

#endif
