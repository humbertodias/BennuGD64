/*
 * PlayStation Vita interpreter entry: clocks, heap, search path, default DCB.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <psp2/power.h>

#include "main_vita.h"
#include "files.h"

/* vitasdk newlib default heap is too small for Bennu + SDL3. */
int _newlib_heap_size_user = 168 * 1024 * 1024;

char * bgdi_vita_startup( int argc, char * argv[], int * standalone )
{
    ( void ) argv;

    scePowerSetArmClockFrequency( 444 );
    scePowerSetGpuClockFrequency( 222 );

    SDL_SetMainReady();
    SDL_SetHint( SDL_HINT_TOUCH_MOUSE_EVENTS, "0" );
#ifdef SDL_HINT_VITA_ENABLE_FRONT_TOUCH
    SDL_SetHint( SDL_HINT_VITA_ENABLE_FRONT_TOUCH, "0" );
#endif
#ifdef SDL_HINT_VITA_ENABLE_BACK_TOUCH
    SDL_SetHint( SDL_HINT_VITA_ENABLE_BACK_TOUCH, "0" );
#endif

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

    chdir( "app0:/" );
    file_addp( "app0:/" );
    file_addp( "ux0:/data/bennugd64/" );
    file_addp( "." );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    if ( access( "app0:/main.dcb", R_OK ) == 0 )
        return "app0:/main.dcb";
    if ( access( "main.dcb", R_OK ) == 0 )
        return "main.dcb";
    if ( access( "ux0:/data/bennugd64/main.dcb", R_OK ) == 0 )
        return "ux0:/data/bennugd64/main.dcb";

    return NULL;
}
