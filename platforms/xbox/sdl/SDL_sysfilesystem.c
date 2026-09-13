/*
 * SDL3 Windows filesystem base paths replaced for nxdk.
 */
#include "SDL_internal.h"

#if defined(SDL_FILESYSTEM_WINDOWS)

#include "../SDL_sysfilesystem.h"

char *SDL_SYS_GetBasePath(void)
{
    return SDL_strdup("D:\\");
}

char *SDL_SYS_GetExeName(void)
{
    return SDL_strdup("default.xbe");
}

char *SDL_SYS_GetPrefPath(const char *org, const char *app)
{
    (void)org;
    (void)app;
    return SDL_strdup("D:\\bennugd64\\");
}

char *SDL_SYS_GetUserFolder(SDL_Folder folder)
{
    (void)folder;
    return SDL_strdup("D:\\");
}

char *SDL_SYS_GetCurrentDirectory(void)
{
    return SDL_strdup("D:\\");
}

#endif /* SDL_FILESYSTEM_WINDOWS */
