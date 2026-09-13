/*
 * SDL3 Windows filesystem ops replaced for nxdk (ANSI-only, no *W APIs).
 * BennuGD uses its own file I/O; SDL FSOPS are unused at runtime.
 */
#include "SDL_internal.h"

#if defined(SDL_FSOPS_WINDOWS)

#include "../SDL_sysfilesystem.h"

bool SDL_SYS_EnumerateDirectory(const char *path, SDL_EnumerateDirectoryCallback cb, void *userdata)
{
    (void)path;
    (void)cb;
    (void)userdata;
    return SDL_Unsupported();
}

bool SDL_SYS_RemovePath(const char *path)
{
    (void)path;
    return SDL_Unsupported();
}

bool SDL_SYS_RenamePath(const char *oldpath, const char *newpath)
{
    (void)oldpath;
    (void)newpath;
    return SDL_Unsupported();
}

bool SDL_SYS_CopyFile(const char *oldpath, const char *newpath)
{
    (void)oldpath;
    (void)newpath;
    return SDL_Unsupported();
}

bool SDL_SYS_CreateDirectory(const char *path)
{
    (void)path;
    return SDL_Unsupported();
}

bool SDL_SYS_GetPathInfo(const char *path, SDL_PathInfo *info)
{
    (void)path;
    (void)info;
    return SDL_Unsupported();
}

#endif /* SDL_FSOPS_WINDOWS */
