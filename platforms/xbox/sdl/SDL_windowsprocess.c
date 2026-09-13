/* SDL process stub for nxdk (no process spawn). */
#include "SDL_internal.h"

#ifdef SDL_PROCESS_WINDOWS

#include "../SDL_sysprocess.h"

bool SDL_SYS_CreateProcessWithProperties(SDL_Process *process, SDL_PropertiesID props)
{
    (void)process;
    (void)props;
    return SDL_Unsupported();
}

bool SDL_SYS_KillProcess(SDL_Process *process, bool force)
{
    (void)process;
    (void)force;
    return SDL_Unsupported();
}

bool SDL_SYS_WaitProcess(SDL_Process *process, bool block, int *exitcode)
{
    (void)process;
    (void)block;
    (void)exitcode;
    return SDL_Unsupported();
}

void SDL_SYS_DestroyProcess(SDL_Process *process)
{
    (void)process;
}

#endif /* SDL_PROCESS_WINDOWS */
