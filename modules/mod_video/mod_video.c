/*
 *  Copyright © 2006-2013 SplinterGU (Fenix/Bennugd)
 *  Copyright © 2002-2006 Fenix Team (Fenix)
 *  Copyright © 1999-2002 José Luis Cebrián Pagüe (Fenix)
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

/* --------------------------------------------------------------------------- */

#include <SDL3/SDL.h>
#include "sdl3_compat.h"

#include <stdlib.h>
#include <stdint.h>

#include "bgdrtm.h"

#include "libgrbase.h"
#include "libvideo.h"
#include "librender.h"

#include "bgddl.h"
#include "dlvaracc.h"

/* --------------------------------------------------------------------------- */

enum {
    GRAPH_MODE = 0
};

/* --------------------------------------------------------------------------- */

DLVARFIXUP __bgdexport( mod_video, globals_fixup )[] =
{
    /* Nombre de variable global, puntero al dato, tamaño del elemento, cantidad de elementos */
    { "graph_mode" , NULL, -1, -1 },
    { NULL , NULL, -1, -1 }
};

/* --------------------------------------------------------------------------- */

/* Funciones de inicialización y carga */

static int modvideo_set_mode( INSTANCE * my, intptr_t * params )
{
    return gr_set_mode( params[0] / 10000, params[0] % 10000, 0 ) ;
}

/* --------------------------------------------------------------------------- */

static int modvideo_set_mode_2( INSTANCE * my, intptr_t * params )
{
    return gr_set_mode( params[0], params[1], 0 ) ;
}

/* --------------------------------------------------------------------------- */

static int modvideo_set_mode_3( INSTANCE * my, intptr_t * params )
{
    GLODWORD( mod_video, GRAPH_MODE ) = (( GLODWORD( mod_video, GRAPH_MODE ) & 0xFF00 ) | params[2] );
    return gr_set_mode( params[0], params[1], 0 ) ;
}

/* --------------------------------------------------------------------------- */

static int modvideo_set_mode_4( INSTANCE * my, intptr_t * params )
{
    GLODWORD( mod_video, GRAPH_MODE ) = ( params[2] | params[3] );
    return gr_set_mode( params[0], params[1], 0 ) ;
}

/* --------------------------------------------------------------------------- */

static int modvideo_set_fps( INSTANCE * my, intptr_t * params )
{
    gr_set_fps( params[0], params[1] ) ;
    return params[0];
}

/* --------------------------------------------------------------------------- */
/*
Return a pointer to an array of available screen dimensions for the given format and video flags,
sorted largest to smallest.

Returns NULL if there are no dimensions available for a particular format,
or -1 if any dimension is okay for the given format.
*/

static int modvideo_list_modes( INSTANCE * my, intptr_t * params )
{
    int i, n = 0;
    static int * available_modes = NULL ;
    SDL_DisplayMode ** modes;
    SDL_DisplayID display_id;

    (void)params; /* depth/flags retained for API compatibility */

    display_id = SDL_GetPrimaryDisplay();
    modes = SDL_GetFullscreenDisplayModes( display_id, &n );
    if ( !modes || n <= 0 )
    {
        if ( modes ) SDL_free( modes );
        return 0;
    }

    available_modes = realloc( available_modes, ( 1 + n ) * sizeof( int ) * 2 );
    if ( !available_modes )
    {
        SDL_free( modes );
        return -2;
    }

    for ( i = 0; i < n; ++i )
    {
        available_modes[i*2  ] = modes[i]->w;
        available_modes[i*2+1] = modes[i]->h;
    }
    available_modes[i*2  ] = 0;
    available_modes[i*2+1] = 0;

    SDL_free( modes );

    return ( int )( intptr_t )available_modes;
}

/* --------------------------------------------------------------------------- */

/*
   returns 0 if the requested mode is not supported under any bit depth,
   or returns the bits-per-pixel of the closest available
   mode with the given width, height and requested flags

   params:
        height,width,depth,flags

*/

static int modvideo_mode_is_ok( INSTANCE * my, intptr_t * params )
{
    int depth = params[2];
    int i, n = 0;
    SDL_DisplayMode ** modes;
    SDL_DisplayID display_id;

    if ( !depth ) depth = ( params[3] & MODE_32BITS ) ? 32 : (( params[3] & MODE_16BITS ) ? 16 : 8 );

    display_id = SDL_GetPrimaryDisplay();
    modes = SDL_GetFullscreenDisplayModes( display_id, &n );
    if ( modes )
    {
        for ( i = 0; i < n; ++i )
        {
            if ( modes[i]->w == params[0] && modes[i]->h == params[1] )
            {
                int bpp = SDL_BITSPERPIXEL( modes[i]->format );
                SDL_free( modes );
                return bpp ? bpp : depth;
            }
        }
        SDL_free( modes );
    }

    /* Windowed modes are always acceptable */
    return depth;
}

/* --------------------------------------------------------------------------- */
/* exports                                                                     */
/* --------------------------------------------------------------------------- */

#include "mod_video_exports.h"

/* --------------------------------------------------------------------------- */
