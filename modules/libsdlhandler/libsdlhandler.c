/*
 *  Copyright © 2006-2013 SplinterGU (Fenix/Bennugd)
 *
 *  This file is part of Bennu - Game Development
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

#include "bgddl.h"

#include <SDL3/SDL.h>
#include "sdl3_compat.h"

#ifdef TARGET_EMSCRIPTEN
#include "libsdlhandler_emscripten.h"
#endif
#ifdef TARGET_WII
#include "libsdlhandler_wii.h"
#endif
#ifdef TARGET_PS2
#include "libsdlhandler_ps2.h"
#endif
#ifdef TARGET_VITA
#include "libsdlhandler_vita.h"
#endif
#ifdef TARGET_PS3
#include "libsdlhandler_ps3.h"
#endif

/* ----------------------------------------------------------------- */
/* Public functions                                                  */

static void  dump_new_events( void )
{
#ifdef TARGET_EMSCRIPTEN
    libsdlhandler_emscripten_pump();
#elif defined(TARGET_PS2)
    libsdlhandler_ps2_pump();
#elif defined(TARGET_VITA)
    libsdlhandler_vita_pump();
#elif defined(TARGET_PS3)
    libsdlhandler_ps3_pump();
#else
    SDL_Event event;
    /* Remove all pendings events */

    /* We can't return -1, just return 0 (no event) on error */
    while ( SDL_PeepEvents( &event, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST ) > 0 );

    /* Get new events */
    SDL_PumpEvents();
#endif
#ifdef TARGET_WII
    libsdlhandler_wii_after_pump();
#endif
}

/* ----------------------------------------------------------------- */
/* Funciones de inicialización del módulo/plugin                     */

void __bgdexport( libsdlhandler, module_initialize )()
{
    /* SDL2 has no SDL_INIT_EVENTTHREAD */
}

/* ----------------------------------------------------------------- */

void __bgdexport( libsdlhandler, module_finalize )()
{
}

/* ----------------------------------------------------------------- */

/* Bigest priority first execute
   Lowest priority last execute */

HOOK __bgdexport( libsdlhandler, handler_hooks )[] =
{
    { 5000, dump_new_events                   },
    {    0, NULL                              }
} ;

/* ----------------------------------------------------------------- */
