/* SDL OpenURL stub for nxdk. */
#include "SDL_internal.h"
#include "../SDL_sysurl.h"

bool SDL_SYS_OpenURL(const char *url)
{
    (void)url;
    return SDL_Unsupported();
}
