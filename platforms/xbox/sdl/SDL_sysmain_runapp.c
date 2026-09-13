/* SDL_RunApp stub for nxdk — Bennu uses its own main. */
#include "SDL_internal.h"

#ifdef SDL_PLATFORM_WIN32

#include "../SDL_main_callbacks.h"

int SDL_RunApp(int argc, char *argv[], SDL_main_func mainFunction, void *reserved)
{
    (void)reserved;
    return SDL_CallMainFunction(argc, argv, mainFunction);
}

#endif
