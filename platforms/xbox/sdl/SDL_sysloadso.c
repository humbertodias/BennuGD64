/* SDL loadso stub for nxdk (static XBE). */
#include "SDL_internal.h"

#ifdef SDL_LOADSO_WINDOWS

SDL_SharedObject *SDL_LoadObject(const char *sofile)
{
    (void)sofile;
    SDL_Unsupported();
    return NULL;
}

SDL_FunctionPointer SDL_LoadFunction(SDL_SharedObject *handle, const char *name)
{
    (void)handle;
    (void)name;
    SDL_Unsupported();
    return NULL;
}

void SDL_UnloadObject(SDL_SharedObject *handle)
{
    (void)handle;
}

#endif /* SDL_LOADSO_WINDOWS */
