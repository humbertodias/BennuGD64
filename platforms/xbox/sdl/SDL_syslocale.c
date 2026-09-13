/* SDL locale stub for nxdk. */
#include "SDL_internal.h"
#include "../SDL_syslocale.h"

bool SDL_SYS_GetPreferredLocales(char *buf, size_t buflen)
{
    if (!buf || buflen == 0) {
        return false;
    }
    SDL_strlcpy(buf, "en_US", buflen);
    return true;
}
