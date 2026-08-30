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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bennugd_git.h"

#include "bgdrtm.h"

#include "bgddl.h"
#include "dlvaracc.h"

#include "libvideo.h"

#ifdef TARGET_PSP
#include "g_video_psp.h"
#endif
#ifdef TARGET_VITA
#include "g_video_vita.h"
#endif
#ifdef TARGET_PS2
#include "g_video_ps2.h"
#endif
#ifdef TARGET_SWITCH
#include "g_video_switch.h"
#endif
#ifdef TARGET_DC
#include "g_video_dc.h"
#endif
#ifdef TARGET_PANDORA
#include "g_video_pandora.h"
#endif
#ifdef TARGET_WII
#include "g_video_wii.h"
#endif
#ifdef TARGET_EMSCRIPTEN
#include "g_video_emscripten.h"
#endif
#ifdef TARGET_WIN32
#include "g_video_win32.h"
#endif

/* --------------------------------------------------------------------------- */

GRAPH * icon = NULL ;

SDL_Window * window = NULL ;
SDL_Surface * screen = NULL ;
SDL_Surface * scale_screen = NULL ;
static SDL_Renderer * present_renderer = NULL ;
static SDL_Texture * present_texture = NULL ;
static int present_tex_w = 0 ;
static int present_tex_h = 0 ;

char * apptitle = NULL ;

int scr_width = 0 ;
int scr_height = 0 ;

int scr_initialized = 0 ;

int enable_16bits = 0 ;
int enable_32bits = 0 ;
int enable_scale = 0 ;
int full_screen = 0 ;
int double_buffer = 0 ;
int hardware_scr = 0 ;
int grab_input = 0 ;
int frameless = 0 ;
int scale_mode = SCALE_NONE ;
int waitvsync = 0 ;

int scale_resolution = -1 ;
int * scale_resolution_table_w = NULL;
int * scale_resolution_table_h = NULL;
int scale_resolution_aspectratio = 0;
int scale_resolution_orientation = 0;

int scale_resolution_aspectratio_offx = 0;
int scale_resolution_aspectratio_offy = 0;

/* --------------------------------------------------------------------------- */

enum {
    GRAPH_MODE = 0,
    SCALE_MODE,
    FULL_SCREEN,
    SCALE_RESOLUTION,
    SCALE_RESOLUTION_ASPECTRATIO,
    SCALE_RESOLUTION_ORIENTATION
};

/* --------------------------------------------------------------------------- */
/* Son las variables que se desea acceder.                           */
/* El interprete completa esta estructura, si la variable existe.    */
/* (usada en tiempo de ejecución)                                    */

DLVARFIXUP __bgdexport( libvideo, globals_fixup )[] =
{
    /* Nombre de variable global, puntero al dato, tamaño del elemento, cantidad de elementos */
    { "graph_mode" , NULL, -1, -1 },
    { "scale_mode" , NULL, -1, -1 },
    { "full_screen" , NULL, -1, -1 },
    { "scale_resolution", NULL, -1, -1 },

    /* new vars for use with scale_resolution */
    { "scale_resolution_aspectratio", NULL, -1, -1 },
    { "scale_resolution_orientation", NULL, -1, -1 },

    { NULL , NULL, -1, -1 }
};

/* --------------------------------------------------------------------------- */

void gr_wait_vsync()
{
#ifdef TARGET_WIN32
    gr_video_win32_wait_vsync();
#endif
}

/* --------------------------------------------------------------------------- */

#ifndef BENNUGD_VERSION
#define BENNUGD_VERSION ""
#endif

static const char * gr_caption_for_window( char * title, char * buf, size_t bufsz )
{
    if ( !BENNUGD_VERSION[0] )
        return title ? title : "";
    if ( title && title[0] )
        snprintf( buf, bufsz, "%s | %s", title, BENNUGD_VERSION );
    else
        snprintf( buf, bufsz, "%s", BENNUGD_VERSION );
    return buf;
}

void gr_set_caption( char * title )
{
    char buf[512];
    apptitle = title ;
    if ( window ) SDL_SetWindowTitle( window, gr_caption_for_window( title, buf, sizeof( buf ) ) ) ;
}

/* --------------------------------------------------------------------------- */

void gr_set_surface_palette( SDL_Surface * surface, SDL_Color * colors, int first, int ncolors )
{
    bennu_set_surface_palette_colors( surface, colors, first, ncolors );
}

/* --------------------------------------------------------------------------- */

static void gr_destroy_present_renderer( void )
{
    if ( present_texture )
    {
        SDL_DestroyTexture( present_texture );
        present_texture = NULL;
    }
    if ( present_renderer )
    {
        SDL_DestroyRenderer( present_renderer );
        present_renderer = NULL;
    }
    present_tex_w = 0;
    present_tex_h = 0;
}

/* Canvas / GLES backends (Emscripten) have no window surface; blit via a renderer. */
int gr_video_present_via_renderer( SDL_Surface * src )
{
    SDL_Surface * converted;
#ifdef TARGET_PS2
    const SDL_PixelFormat fmt = SDL_PIXELFORMAT_ABGR1555;
#else
    const SDL_PixelFormat fmt = SDL_PIXELFORMAT_XRGB8888;
#endif

    if ( !present_renderer )
    {
#ifdef TARGET_WII
        present_renderer = SDL_CreateRenderer( window, "OGC EFB" );
        if ( !present_renderer )
#endif
#ifdef TARGET_PS2
        present_renderer = SDL_CreateRenderer( window, "PS2 gsKit" );
        if ( !present_renderer )
#endif
        present_renderer = SDL_CreateRenderer( window, NULL );
        if ( !present_renderer ) return 0;
        /* rAF already paces the interpreter on the web; extra vsync here
         * waits a second display tick and halves the game speed. */
        SDL_SetRenderVSync( present_renderer, false );
    }

    if ( !present_texture || present_tex_w != src->w || present_tex_h != src->h )
    {
        if ( present_texture ) SDL_DestroyTexture( present_texture );
        present_texture = SDL_CreateTexture( present_renderer, fmt, SDL_TEXTUREACCESS_STREAMING, src->w, src->h );
        if ( !present_texture ) return 0;
        present_tex_w = src->w;
        present_tex_h = src->h;
        SDL_SetTextureScaleMode( present_texture, SDL_SCALEMODE_NEAREST );
    }

    converted = src;
    if ( src->format != fmt )
    {
        converted = SDL_ConvertSurface( src, fmt );
        if ( !converted ) return 0;
    }

    SDL_UpdateTexture( present_texture, NULL, converted->pixels, converted->pitch );
    if ( converted != src ) SDL_DestroySurface( converted );

    SDL_SetRenderDrawColor( present_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( present_renderer );
    SDL_RenderTexture( present_renderer, present_texture, NULL, NULL );
    SDL_RenderPresent( present_renderer );
    return 1;
}

void gr_video_present( SDL_Surface * src )
{
    SDL_Surface * winsurf ;

    if ( !window || !src ) return ;

#ifdef TARGET_SWITCH
    gr_video_switch_present( src );
    return;
#endif
#ifdef TARGET_PSP
    gr_video_psp_present( src );
    return;
#endif
#ifdef TARGET_VITA
    gr_video_vita_present( src );
    return;
#endif
#ifdef TARGET_PS2
    gr_video_ps2_present( src );
    return;
#endif
#ifdef TARGET_DC
    gr_video_dc_present( src );
    return;
#endif
#ifdef TARGET_WII
    gr_video_wii_present( src );
    return;
#endif

    winsurf = SDL_GetWindowSurface( window );
    if ( !winsurf )
    {
        gr_video_present_via_renderer( src );
        return;
    }

    if ( winsurf->w == src->w && winsurf->h == src->h )
        SDL_BlitSurface( src, NULL, winsurf, NULL );
    else
        SDL_BlitSurfaceScaled( src, NULL, winsurf, NULL, SDL_SCALEMODE_NEAREST );

    SDL_UpdateWindowSurface( window );
}

/* --------------------------------------------------------------------------- */

void gr_video_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    SDL_Surface * winsurf ;
    int i ;

    if ( !window || !src || count <= 0 ) return ;

#ifdef TARGET_SWITCH
    gr_video_switch_present_rects( src, rects, count );
    return;
#endif
#ifdef TARGET_PSP
    gr_video_psp_present_rects( src, rects, count );
    return;
#endif
#ifdef TARGET_VITA
    gr_video_vita_present_rects( src, rects, count );
    return;
#endif
#ifdef TARGET_PS2
    gr_video_ps2_present_rects( src, rects, count );
    return;
#endif
#ifdef TARGET_WII
    gr_video_wii_present_rects( src, rects, count );
    return;
#endif

    winsurf = SDL_GetWindowSurface( window );
    if ( !winsurf )
    {
        gr_video_present( src );
        return ;
    }

    /* Scaled windows can't map dirty rects 1:1; refresh the whole frame. */
    if ( winsurf->w != src->w || winsurf->h != src->h )
    {
        gr_video_present( src );
        return ;
    }

    for ( i = 0 ; i < count ; i++ )
        SDL_BlitSurface( src, ( SDL_Rect * ) &rects[ i ], winsurf, ( SDL_Rect * ) &rects[ i ] );

    SDL_UpdateWindowSurfaceRects( window, rects, count );
}

/* --------------------------------------------------------------------------- */

static SDL_Surface * gr_create_shadow_surface( int width, int height, int depth )
{
    Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0 ;

    if ( depth == 16 )
    {
        rmask = 0xF800 ;
        gmask = 0x07E0 ;
        bmask = 0x001F ;
    }
    else if ( depth == 32 )
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xFF000000 ;
        gmask = 0x00FF0000 ;
        bmask = 0x0000FF00 ;
        amask = 0x000000FF ;
#else
        rmask = 0x000000FF ;
        gmask = 0x0000FF00 ;
        bmask = 0x00FF0000 ;
        amask = 0xFF000000 ;
#endif
    }

    return bennu_create_rgb_surface( width, height, depth, rmask, gmask, bmask, amask );
}

/* --------------------------------------------------------------------------- */

static int gr_setup_sdl_window( int width, int height, Uint32 window_flags )
{
    int cur_w = 0, cur_h = 0;
    int recreate = 0;
    char caption_buf[512];

#ifdef TARGET_SWITCH
    gr_video_switch_adjust_window( &width, &height, &window_flags );
#endif
#ifdef TARGET_DC
    gr_video_dc_adjust_window( &width, &height, &window_flags );
#endif
#ifdef TARGET_PSP
    gr_video_psp_adjust_window( &width, &height, &window_flags );
#endif
#ifdef TARGET_VITA
    gr_video_vita_adjust_window( &width, &height, &window_flags );
#endif
#ifdef TARGET_PS2
    gr_video_ps2_adjust_window( &width, &height, &window_flags );
    gr_video_ps2_before_window();
#endif
#ifdef TARGET_PANDORA
    gr_video_pandora_adjust_window( &width, &height, &window_flags );
#endif
#ifdef TARGET_WII
    gr_video_wii_adjust_window( &width, &height, &window_flags );
#endif

    if ( !window )
    {
        recreate = 1;
    }
    else
    {
        SDL_GetWindowSize( window, &cur_w, &cur_h );
        if ( cur_w != width || cur_h != height )
            recreate = 1;
        if ( !!( SDL_GetWindowFlags( window ) & SDL_WINDOW_FULLSCREEN ) != !!full_screen )
            recreate = 1;
#ifdef TARGET_EMSCRIPTEN
        if ( !gr_video_emscripten_should_recreate_window() )
            recreate = 0;
#endif
    }

    if ( recreate )
    {
        if ( window )
        {
            gr_destroy_present_renderer();
#ifdef TARGET_PSP
            gr_video_psp_destroy();
#endif
#ifdef TARGET_WII
            gr_video_wii_destroy();
#endif
            SDL_DestroyWindow( window );
            window = NULL;
        }
        window = SDL_CreateWindow( gr_caption_for_window( apptitle, caption_buf, sizeof( caption_buf ) ), width, height, window_flags );
#ifdef TARGET_PS2
        gr_video_ps2_after_window();
#endif
        if ( !window ) return -1;
    }
    else
    {
        SDL_SetWindowFullscreen( window, full_screen ? true : false );
        SDL_SetWindowSize( window, width, height );
    }

    SDL_SetWindowBordered( window, ( frameless || full_screen ) ? false : true );
    SDL_SetWindowResizable( window, ( frameless || full_screen ) ? false : true );

    if ( !full_screen )
    {
        SDL_RestoreWindow( window );
        SDL_SetWindowSize( window, width, height );
        SDL_SetWindowPosition( window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
    }

    return 0;
}

/* --------------------------------------------------------------------------- */

int gr_set_icon( GRAPH * map )
{
    if (( icon = map ))
    {
        SDL_Surface *ico = NULL;
        if ( icon->format->depth == 8 )
        {
            SDL_Color palette[256];
            if ( sys_pixel_format && sys_pixel_format->palette )
            {
                int n ;
                for ( n = 0 ; n < 256 ; n++ )
                {
                    palette[ n ].r = sys_pixel_format->palette->rgb[ n ].r;
                    palette[ n ].g = sys_pixel_format->palette->rgb[ n ].g;
                    palette[ n ].b = sys_pixel_format->palette->rgb[ n ].b;
                    palette[ n ].a = 255;
                }
            }

            ico = bennu_create_rgb_surface_from( icon->data, 32, 32, 8, 32, 0x00, 0x00, 0x00, 0x00 ) ;
            gr_set_surface_palette( ico, palette, 0, 256 );
        }
        else
        {
            ico = bennu_create_rgb_surface_from( icon->data, 32, 32, icon->format->depth, icon->pitch, icon->format->Rmask, icon->format->Gmask, icon->format->Bmask, icon->format->Amask ) ;
        }

        SDL_SetColorKey( ico, SDL_TRUE, bennu_map_rgb( ico, 0, 0, 0 ) ) ;
        if ( window ) SDL_SetWindowIcon( window, ico );
        SDL_FreeSurface( ico ) ;
    }

    return 1 ;
}

/* --------------------------------------------------------------------------- */

int gr_set_mode( int width, int height, int depth )
{
    int n ;
    int surface_width;
    int surface_height;
    Uint32 window_flags = 0;
    char * e;

    /* SDL_CreateSurface(0,0) is invalid. Do not fall back to the desktop
     * size: that opens a display-sized window, SetWindowSize often cannot
     * shrink it (Wayland), and the present path then stretches the game. */
    if ( width < 1 ) width = 320;
    if ( height < 1 ) height = 200;

    surface_width = width;
    surface_height = height;

    enable_scale = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_2XSCALE ) ? 1 : 0 ;
    full_screen = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_FULLSCREEN ) ? 1 : 0 ;
    double_buffer = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_DOUBLEBUFFER ) ? 1 : 0 ;
    hardware_scr = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_HARDWARE ) ? 1 : 0 ;
    grab_input = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_MODAL ) ? 1 : 0 ;
    frameless = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_FRAMELESS ) ? 1 : 0 ;
    waitvsync = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_WAITVSYNC ) ? 1 : 0 ;
    scale_mode = GLODWORD( libvideo, SCALE_MODE );
    full_screen |= GLODWORD( libvideo, FULL_SCREEN );
#ifdef TARGET_SWITCH
    gr_video_switch_apply_mode();
#endif
#ifdef TARGET_DC
    gr_video_dc_apply_mode();
#endif
#ifdef TARGET_PSP
    gr_video_psp_apply_mode();
#endif
#ifdef TARGET_VITA
    gr_video_vita_apply_mode();
    /* GXM present is XRGB8888. hello.prg's set_mode(..., 16) would otherwise
     * fill colorghost[65536] and ConvertSurface every FRAME. */
    depth = 32;
#endif
#ifdef TARGET_PS2
    gr_video_ps2_apply_mode();
    GLODWORD( libvideo, SCALE_RESOLUTION ) = -1;
    GLODWORD( libvideo, GRAPH_MODE ) = MODE_16BITS | MODE_FULLSCREEN;
    depth = 16;
    enable_scale = 0;
#endif
#ifdef TARGET_PANDORA
    gr_video_pandora_apply_mode();
#endif
#ifdef TARGET_WII
    gr_video_wii_apply_mode();
#endif

    scale_resolution = GLODWORD( libvideo, SCALE_RESOLUTION );

    if ( GLOEXISTS( libvideo, SCALE_RESOLUTION_ASPECTRATIO ) ) scale_resolution_aspectratio = GLODWORD( libvideo, SCALE_RESOLUTION_ASPECTRATIO );
    if ( GLOEXISTS( libvideo, SCALE_RESOLUTION_ORIENTATION ) ) scale_resolution_orientation = GLODWORD( libvideo, SCALE_RESOLUTION_ORIENTATION );

#ifndef TARGET_PS2
    /* Overwrite all params */
    if ( ( e = getenv( "SCALE_RESOLUTION"             ) ) ) scale_resolution = atol( e );
    if ( ( e = getenv( "SCALE_RESOLUTION_ASPECTRATIO" ) ) ) scale_resolution_aspectratio = atol( e );
    if ( ( e = getenv( "SCALE_RESOLUTION_ORIENTATION" ) ) ) scale_resolution_orientation = atol( e );
#endif

    if ( scale_resolution_orientation < 0 || scale_resolution_orientation > 4 ) scale_resolution_orientation = 0;

    if ( !depth )
    {
        enable_32bits = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_32BITS ) ? 1 : 0 ;
        if ( !enable_32bits )
            enable_16bits = ( GLODWORD( libvideo, GRAPH_MODE ) & MODE_16BITS ) ? 1 : 0 ;
        else
            enable_16bits = 0;
        depth = enable_32bits ? 32 : ( enable_16bits ? 16 : 8 );
    }
    else if ( depth == 16 )
    {
        enable_16bits = 1;
        enable_32bits = 0;
    }
    else if ( depth == 32 )
    {
        enable_16bits = 0;
        enable_32bits = 1;
    }

    if ( scale_resolution_table_w )
    {
        free( scale_resolution_table_w );
        scale_resolution_table_w = NULL;
    }

    if ( scale_resolution_table_h )
    {
        free( scale_resolution_table_h );
        scale_resolution_table_h = NULL;
    }

    if ( scale_resolution != -1 )
    {
        surface_width  = scale_resolution / 10000 ;
        surface_height = scale_resolution % 10000 ;
    }
    else
    {
        if ( scale_mode != SCALE_NONE ) enable_scale = 1;
        if ( enable_scale && scale_mode == SCALE_NONE ) scale_mode = SCALE_SCALE2X;

        if ( enable_scale )
        {
            enable_16bits = 1;
            depth = 16;

            surface_width  *= 2;
            surface_height *= 2;
        }
    }

    /* Inicializa el modo gráfico */

    if ( scrbitmap )
    {
        bitmap_destroy( scrbitmap ) ;
        scrbitmap = NULL ;
    }

    /* Setup the SDL Window + software surfaces */

    if ( full_screen ) window_flags |= SDL_WINDOW_FULLSCREEN;
    if ( frameless ) window_flags |= SDL_WINDOW_BORDERLESS;
    /* Resizable so window managers expose minimize/maximize/close chrome. */
    if ( !full_screen && !frameless ) window_flags |= SDL_WINDOW_RESIZABLE;

    if ( scale_screen )
    {
        SDL_FreeSurface( scale_screen );
        scale_screen = NULL;
    }
    if ( screen )
    {
        SDL_FreeSurface( screen );
        screen = NULL;
    }

    if ( scale_resolution != -1 )
    {
        switch ( scale_resolution_orientation )
        {
            case    SRO_LEFT:
            case    SRO_RIGHT:
            {
                    int aux =  surface_width;
                    surface_width = surface_height;
                    surface_height = aux;
                    break;
            }
        }

        if ( gr_setup_sdl_window( surface_width, surface_height, window_flags ) < 0 )
            return -1;

        scale_screen = gr_create_shadow_surface( surface_width, surface_height, depth );

        if ( !scale_screen ) return -1;

        if ( !width && !height )
        {
            width = scale_screen->w;
            height = scale_screen->h;
        }

        screen = gr_create_shadow_surface( width, height, bennu_surface_bpp( scale_screen ) );

        /* scale tables */

        int     lim_w = 0, lim_h = 0, pitch_w = 0, pitch_h = 0;
        double  fw = 0.0, fh = 0.0, fx = 0.0, fy = 0.0;
        int     h, w;
        int     start_w = 0, start_h = 0, fix = 1;

        scale_resolution_aspectratio_offx = 0;
        scale_resolution_aspectratio_offy = 0;

        switch ( scale_resolution_orientation )
        {
            case    SRO_NORMAL:
            case    SRO_DOWN:
                    lim_w = screen->w;
                    lim_h = screen->h;

                    pitch_w = 1;
                    pitch_h = screen->pitch;

                    fw = (double)screen->w / (double)scale_screen->w;
                    fh = (double)screen->h / (double)scale_screen->h;
                    break;

            case    SRO_LEFT:
            case    SRO_RIGHT:
                    lim_w = screen->h;
                    lim_h = screen->w;

                    pitch_w = screen->pitch;
                    pitch_h = 1;

                    fh = (double)screen->w / (double)scale_screen->h;
                    fw = (double)screen->h / (double)scale_screen->w;
                    break;
        }

        switch ( scale_resolution_orientation )
        {
            case    SRO_NORMAL:
            case    SRO_LEFT:
                    start_w = 0;
                    start_h = 0;
                    fix = -1;
                    break;

            case    SRO_DOWN:
            case    SRO_RIGHT:
                    start_w = scale_screen->w - 1;
                    start_h = scale_screen->h - 1;
                    fix = 1;
                    break;
        }

        if ( scale_resolution_aspectratio == SRA_PRESERVE )
        {
            if ( scale_screen->w > scale_screen->h )
            {
                fw = fh;
                scale_resolution_aspectratio_offx = ( scale_screen->w - lim_w / fw ) / 2 ;
                scale_resolution_aspectratio_offy = 0;
            }
            else
            {
                fh = fw;
                scale_resolution_aspectratio_offx = 0;
                scale_resolution_aspectratio_offy = ( scale_screen->h - lim_h / fh ) / 2 ;
            }
        }

        if ( !( scale_resolution_table_w = malloc( scale_screen->w * sizeof( int ) ) ) ) return -1;
        if ( !( scale_resolution_table_h = malloc( scale_screen->h * sizeof( int ) ) ) ) return -1;

        for ( w = 0; w < scale_screen->w; w++ )
        {
            if ( w < scale_resolution_aspectratio_offx )
                scale_resolution_table_w[ start_w - w * fix ] = -1;
            else
            {
                scale_resolution_table_w[ start_w - w * fix ] = ( fx < lim_w ) ? pitch_w * ( int ) fx : -1 ;
                fx += fw;
            }
        }

        for ( h = 0; h < scale_screen->h; h++ )
        {
            if ( h < scale_resolution_aspectratio_offy )
                scale_resolution_table_h[ start_h - h * fix ] = -1;
            else
            {
                scale_resolution_table_h[ start_h - h * fix ] = ( fy < lim_h ) ? pitch_h * ( int ) fy : -1 ;
                fy += fh;
            }
        }
    }
    else
    {
        if ( gr_setup_sdl_window( surface_width, surface_height, window_flags ) < 0 )
            return -1;

        screen = gr_create_shadow_surface( surface_width, surface_height, depth );
    }

    if ( !screen ) return -1;

#ifdef TARGET_PSP
    if ( !gr_video_psp_ready_present( screen->w, screen->h ) ) return -1;
#endif

    SDL_SetWindowMouseGrab( window, grab_input ? true : false ) ;
    SDL_SetWindowKeyboardGrab( window, grab_input ? true : false ) ;

    /* Set window title */
    gr_set_caption( apptitle ) ;

    if ( !sys_pixel_format )
    {
        sys_pixel_format = bitmap_create_format( depth );
    }
    else
    {
        PALETTE * p = sys_pixel_format->palette;

        free( sys_pixel_format );
        sys_pixel_format = bitmap_create_format( depth );

        if ( p )
        {
            sys_pixel_format->palette = p;
            pal_refresh( sys_pixel_format->palette ) ;
        }
    }

    if ( sys_pixel_format->depth == 16 )
    {
        Uint32 rmask = bennu_surface_rmask( screen );
        Uint32 gmask = bennu_surface_gmask( screen );
        Uint32 bmask = bennu_surface_bmask( screen );

        for ( n = 0 ; n < 65536 ; n++ )
        {
            colorghost[ n ] =
                ((( n & rmask ) >> 1 ) & rmask ) +
                ((( n & gmask ) >> 1 ) & gmask ) +
                ((( n & bmask ) >> 1 ) & bmask ) ;
        }
    }

    scr_initialized = 1 ;

    SDL_HideCursor() ;

    pal_refresh( NULL ) ;
    palette_changed = 1 ;

//    gr_make_trans_table();

    /* With classic 2x scale, SDL screen is physical (2x) while game coordinates,
     * background, regions and dirty-rects stay at the logical resolution. */
    {
        int logical_w = screen->w ;
        int logical_h = screen->h ;

        if ( scale_resolution == -1 && enable_scale )
        {
            logical_w = screen->w / 2 ;
            logical_h = screen->h / 2 ;
        }

        /* Bitmaps de fondo */

        /* Only allow background with same properties that video mode */
        if (
            !background ||
            background->width != ( uint32_t ) logical_w ||
            background->height != ( uint32_t ) logical_h ||
            sys_pixel_format->depth != background->format->depth )
        {
            if ( background ) bitmap_destroy( background );
            background = bitmap_new( 0, logical_w, logical_h, sys_pixel_format->depth ) ;
            if ( background )
            {
                gr_clear( background ) ;
                bitmap_add_cpoint( background, 0, 0 ) ;
            }
        }

        scr_width = logical_w ;
        scr_height = logical_h ;

        regions[0].x  = 0 ;
        regions[0].y  = 0 ;
        regions[0].x2 = logical_w - 1 ;
        regions[0].y2 = logical_h - 1 ;
    }

    gr_set_icon( icon );

    if ( background ) background->modified = 1;

#ifdef TARGET_EMSCRIPTEN
    gr_video_emscripten_after_set_mode();
#endif

    return 0;
}

/* --------------------------------------------------------------------------- */

int gr_init( int width, int height )
{
    return gr_set_mode( width, height, 0 );
}

/* --------------------------------------------------------------------------- */

void __bgdexport( libvideo, module_initialize )()
{
#ifdef TARGET_PS2
    /* SET_MODE opens gsKit. Do not touch SDL or GRAPH globals here. */
    apptitle = appname;
    return;
#endif
    char * e;

    GLODWORD( libvideo, SCALE_RESOLUTION ) = -1; // hack for backward compatibility

#ifdef TARGET_EMSCRIPTEN
    gr_video_emscripten_module_initialize();
#endif
#ifdef TARGET_SWITCH
    gr_video_switch_module_initialize();
#endif
#ifdef TARGET_DC
    gr_video_dc_module_initialize();
#endif
#ifdef TARGET_WII
    gr_video_wii_module_initialize();
#endif
#ifdef TARGET_PSP
    gr_video_psp_module_initialize();
#elif defined(TARGET_VITA)
    gr_video_vita_module_initialize();
#elif defined(TARGET_PANDORA)
    gr_video_pandora_module_initialize();
#else
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_InitSubSystem( SDL_INIT_VIDEO );
#endif

#ifdef TARGET_WIN32
    gr_video_win32_module_initialize();
#endif
    apptitle = appname;

    if ( ( e = getenv( "VIDEO_WIDTH"  ) ) ) scr_width = atoi(e);
    if ( ( e = getenv( "VIDEO_HEIGHT" ) ) ) scr_height = atoi(e);
    if ( ( e = getenv( "VIDEO_DEPTH"  ) ) )
        GLODWORD( libvideo, GRAPH_MODE ) = atoi(e);
    else
        GLODWORD( libvideo, GRAPH_MODE ) = MODE_16BITS;
    if ( ( e = getenv( "VIDEO_FULLSCREEN" ) ) ) GLODWORD( libvideo, GRAPH_MODE ) |= atoi(e) ? MODE_FULLSCREEN : 0;

    gr_init( scr_width, scr_height ) ;
}

/* --------------------------------------------------------------------------- */

void __bgdexport( libvideo, module_finalize )()
{
#ifdef TARGET_WIN32
    gr_video_win32_module_finalize();
#endif
    if ( scale_screen )
    {
        SDL_FreeSurface( scale_screen );
        scale_screen = NULL;
    }
    if ( screen )
    {
        SDL_FreeSurface( screen );
        screen = NULL;
    }
    if ( window )
    {
        gr_destroy_present_renderer();
#ifdef TARGET_PSP
        gr_video_psp_destroy();
#endif
#ifdef TARGET_WII
        gr_video_wii_destroy();
#endif
        SDL_DestroyWindow( window );
        window = NULL;
    }
    if ( SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

/* --------------------------------------------------------------------------- */
/* exports                                                                     */
/* --------------------------------------------------------------------------- */

#include "libvideo_exports.h"

/* --------------------------------------------------------------------------- */
