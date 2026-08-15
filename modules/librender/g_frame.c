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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "librender.h"

/* --------------------------------------------------------------------------- */

#define FPS_INTIAL_VALUE    25
#define FPS_INTIAL_SKIP     2

/* --------------------------------------------------------------------------- */

int fps_value = FPS_INTIAL_VALUE ;
int max_jump = FPS_INTIAL_SKIP ;
float frame_ms = 1000.0 / FPS_INTIAL_VALUE ; /* 40.0 ; */

uint32_t frame_count = 0 ;
int last_frame_ticks = 0 ;
int jump = 0 ;

int FPS_count = 0 ;
int FPS_init = 0 ;

int FPS_count_sync = 0 ;
int FPS_init_sync = 0 ;
static int fps_started = 0 ;

float ticks_per_frame = 0;
float fps_partial = 0;

/* --------------------------------------------------------------------------- */
/* Inicialización y controles de tiempo                                        */
/* --------------------------------------------------------------------------- */

/*
 *  FUNCTION : gr_set_fps
 *
 *  Change the game fps and frameskip values
 *
 *  PARAMS :
 *      fps         New number of frames per second
 *      jump        New value of maximum frameskip
 *
 *  RETURN VALUE :
 *      None
 */

void gr_set_fps( int fps, int skip )
{
    if ( fps == fps_value && skip == max_jump ) return ;

    frame_ms = fps ? 1000.0 / ( float ) fps : 0.0 ;
    max_jump = skip ;
    fps_value = ( int ) fps;

    FPS_init_sync = FPS_init = 0 ;
    FPS_count_sync = FPS_count = 0 ;
    fps_started = 0 ;

    jump = 0;

    bgdrtm_set_frame_pace( fps_value );
}

static int bennu_get_ticks_ms( void )
{
#if defined(TARGET_GP2X_WIZ) || defined(TARGET_CAANOO)
    return ( int )( bgdrtm_ptimer_get_ticks_us() / 1000L );
#else
    return ( int ) SDL_GetTicks();
#endif
}

#ifndef __EMSCRIPTEN__
static void bennu_delay_ms( int delay )
{
    if ( delay > 0 )
        SDL_Delay( delay );
}
#endif

/* --------------------------------------------------------------------------- */
/*
 *  FUNCTION : gr_wait_frame
 *
 *  Wait for the next frame start.
 *
 *  PARAMS :
 *      None
 *
 *  RETURN VALUE :
 *      None
 */

void gr_wait_frame()
{
    int frame_ticks ;

    frame_count++ ;

    /* -------------- */

    /* Tomo Tick actual. Do not treat tick 0 as "uninitialized" — on
     * Emscripten SDL_GetTicks() often starts at 0, which skipped the limiter. */
    frame_ticks = bennu_get_ticks_ms();
    if ( !fps_started )
    {
        fps_started = 1;
        FPS_init_sync = FPS_init = frame_ticks;
        FPS_count_sync = FPS_count = 0 ;
        jump = 0;

        /* Tiempo inicial del nuevo frame */
        last_frame_ticks = frame_ticks ;

        return;
    }

    /* Tiempo transcurrido total del ejecucion del ultimo frame (Frame time en ms) */
    * ( float * ) &GLODWORD( librender, FRAME_TIME ) = ( frame_ticks - last_frame_ticks ) / 1000.0f ;

    /* -------------- */

    FPS_count++ ;

    /* -------------- */

    if ( fps_value )
    {
        FPS_count_sync++ ;

        ticks_per_frame = ( ( float ) ( frame_ticks - FPS_init_sync ) ) / ( float ) FPS_count_sync ;
        fps_partial = 1000.0 / ticks_per_frame ;

        if ( fps_partial == fps_value )
        {
            FPS_init_sync = frame_ticks ;
            FPS_count_sync = 0 ;
            jump = 0;
        }
        else if ( fps_partial > fps_value )
        {
            int delay = FPS_count_sync * frame_ms - ( frame_ticks - FPS_init_sync ) ;

            if ( delay > 0 )
            {
#if defined(TARGET_GP2X_WIZ) || defined(TARGET_CAANOO)
                {
                    unsigned long ta = bgdrtm_ptimer_get_ticks_us(), te = ta + delay * 1000;
                    if ( ta > te ) while ( bgdrtm_ptimer_get_ticks_us() > te );
                    while ( bgdrtm_ptimer_get_ticks_us() < te );
                }
#elif !defined(__EMSCRIPTEN__)
                bennu_delay_ms( delay ) ;
                /* Reajust after delay */
                frame_ticks = bennu_get_ticks_ms();
                ticks_per_frame = ( ( float ) ( frame_ticks - FPS_init_sync ) ) / ( float ) FPS_count_sync ;
                fps_partial = 1000.0 / ticks_per_frame ;
#endif
            }

            jump = 0 ;
        }
        else
        {
            if ( jump < max_jump ) /* Como no me alcanza el tiempo, voy a hacer skip */
                jump++ ; /* No dibujar el frame */
            else
            {
                FPS_init_sync = frame_ticks ;
                FPS_count_sync = 0 ;
                jump = 0 ;
            }
        }
    }

    /* Si paso 1 segundo o mas desde la ultima lectura */
    if ( frame_ticks - FPS_init >= 1000 )
    {
        if ( fps_value )
        {
            GLODWORD( librender, SPEED_GAUGE ) = FPS_count /*fps_partial*/ * 100.0 / fps_value ;
        }
        else
        {
            GLODWORD( librender, SPEED_GAUGE ) = 100 ;
        }

        GLODWORD( librender, FPS ) = FPS_count ;

        FPS_init = frame_ticks ;
        FPS_count = 0 ;
    }

    /* Tiempo inicial del nuevo frame */
    last_frame_ticks = frame_ticks ;
}

/* --------------------------------------------------------------------------- */

static SDL_Color palette[256];

void gr_refresh_palette()
{
    int n ;

    if ( sys_pixel_format->depth > 8 )
    {
        if ( sys_pixel_format->palette )
        {
            for ( n = 0 ; n < 256 ; n++ )
            {
                sys_pixel_format->palette->colorequiv[ n ] = _rgb(
                                                                  sys_pixel_format,
                                                                  sys_pixel_format->palette->rgb[ n ].r,
                                                                  sys_pixel_format->palette->rgb[ n ].g,
                                                                  sys_pixel_format->palette->rgb[ n ].b
                                                                 ) ;
            }
        }
    }
    else if ( sys_pixel_format->depth == 8 )
    {
        if ( sys_pixel_format->palette )
        {
            for ( n = 0 ; n < 256 ; n++ )
            {
                palette[ n ].r = sys_pixel_format->palette->rgb[ n ].r;
                palette[ n ].g = sys_pixel_format->palette->rgb[ n ].g;
                palette[ n ].b = sys_pixel_format->palette->rgb[ n ].b;
            }
        }
        else
        {
            uint8_t * pal = default_palette;
            for ( n = 0 ; n < 256 ; n++ )
            {
                palette[ n ].r = *pal++;
                palette[ n ].g = *pal++;
                palette[ n ].b = *pal++;
            }
        }
        if ( scale_screen )
            gr_set_surface_palette( scale_screen, palette, 0, 256 ) ;
        else
            gr_set_surface_palette( screen, palette, 0, 256 ) ;
    }

    palette_changed = 0;
    trans_table_updated = 0 ;
}

/* --------------------------------------------------------------------------- */

void gr_draw_frame()
{
    if ( jump ) return ;

    /* Actualiza paleta */

    if ( palette_changed ) gr_refresh_palette();

    if ( !trans_table_updated ) gr_make_trans_table();

    /* Bloquea el bitmap de pantalla */

    if ( gr_lock_screen() < 0 ) return ;

    /* Dibuja la pantalla */

    gr_draw_screen( scrbitmap, GLODWORD( librender, RESTORETYPE ), GLODWORD( librender, DUMPTYPE ) );

    /* Fading */

    if ( fade_on || fade_set )
    {
        gr_fade_step() ;
        if ( background ) background->modified = 1 ;
    }

    /* Actualiza la paleta y la pantalla */

    gr_unlock_screen() ;

}

/* --------------------------------------------------------------------------- */

void __bgdexport( librender, module_initialize )()
{
    /* SDL3: timers are always available; SDL_INIT_TIMER was removed */
    bgdrtm_set_frame_pace( fps_value );
}

/* --------------------------------------------------------------------------- */

void __bgdexport( librender, module_finalize )()
{
}

/* --------------------------------------------------------------------------- */
