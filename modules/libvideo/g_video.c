/*
 *  Copyright � 2006-2013 SplinterGU (Fenix/Bennugd)
 *  Copyright � 2002-2006 Fenix Team (Fenix)
 *  Copyright � 1999-2002 Jos� Luis Cebri�n Pag�e (Fenix)
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

#include <stdlib.h>
#include <string.h>

#include "bgdrtm.h"

#include "bgddl.h"
#include "dlvaracc.h"

#include "libvideo.h"

#ifdef _WIN32
#include <initguid.h>
#include "ddraw.h"
#endif

/* --------------------------------------------------------------------------- */

GRAPH * icon = NULL ;

SDL_Window * window = NULL ;
SDL_Surface * screen = NULL ;
SDL_Surface * scale_screen = NULL ;

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
/* (usada en tiempo de ejecucion)                                    */

DLVARFIXUP __bgdexport( libvideo, globals_fixup )[] =
{
    /* Nombre de variable global, puntero al dato, tama�o del elemento, cantidad de elementos */
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

#ifdef _WIN32
/* Based allegro */

LPDIRECTDRAW2 directdraw = NULL;
DDCAPS ddcaps;

HRESULT WINAPI( *_DirectDrawCreate )( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );

/* --------------------------------------------------------------------------- */

int init_dx( void )
{
    HINSTANCE handle;
    LPDIRECTDRAW directdraw1;
    HRESULT hr;
    LPVOID temp;

    handle = LoadLibrary( "DDRAW.DLL" );
    if ( handle == NULL ) return -1;

    _DirectDrawCreate = GetProcAddress( handle, "DirectDrawCreate" );

    hr = _DirectDrawCreate( NULL, &directdraw1, NULL );
    if ( FAILED( hr ) ) return -1;

    hr = IDirectDraw_QueryInterface( directdraw1, &IID_IDirectDraw2, &directdraw );
    if ( FAILED( hr ) ) return -1;

    IDirectDraw_Release( directdraw1 );

    hr = IDirectDraw2_SetCooperativeLevel( directdraw, NULL, DDSCL_NORMAL );
    if ( FAILED( hr ) ) return -1;

    /* get capabilities */
    ddcaps.dwSize = sizeof( ddcaps );
    hr = IDirectDraw2_GetCaps( directdraw, &ddcaps, NULL );
    if ( FAILED( hr ) ) return -1;

    return 0;
}
#endif

/* --------------------------------------------------------------------------- */

void gr_wait_vsync()
{
#ifdef _WIN32
    if ( directdraw ) IDirectDraw2_WaitForVerticalBlank( directdraw, DDWAITVB_BLOCKBEGIN, NULL );
#endif
}

/* --------------------------------------------------------------------------- */

void gr_set_caption( char * title )
{
    apptitle = title ;
    if ( window ) SDL_SetWindowTitle( window, title ? title : "" ) ;
}

/* --------------------------------------------------------------------------- */

void gr_set_surface_palette( SDL_Surface * surface, SDL_Color * colors, int first, int ncolors )
{
    bennu_set_surface_palette_colors( surface, colors, first, ncolors );
}

/* --------------------------------------------------------------------------- */

void gr_video_present( SDL_Surface * src )
{
    SDL_Surface * winsurf ;

    if ( !window || !src ) return ;

    winsurf = SDL_GetWindowSurface( window );
    if ( !winsurf ) return ;

    SDL_BlitSurface( src, NULL, winsurf, NULL );
    SDL_UpdateWindowSurface( window );
}

/* --------------------------------------------------------------------------- */

void gr_video_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    SDL_Surface * winsurf ;
    int i ;

    if ( !window || !src || count <= 0 ) return ;

    winsurf = SDL_GetWindowSurface( window );
    if ( !winsurf ) return ;

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

    /* SDL1 SetVideoMode(0,0) used the desktop size. SDL2/3 CreateSurface(0,0)
     * yields an empty surface and later crashes in gr_lock_screen. */
    if ( width < 1 || height < 1 )
    {
        const SDL_DisplayMode * mode = SDL_GetDesktopDisplayMode( SDL_GetPrimaryDisplay() );
        if ( mode )
        {
            if ( width < 1 ) width = mode->w;
            if ( height < 1 ) height = mode->h;
        }
        if ( width < 1 ) width = 320;
        if ( height < 1 ) height = 200;
    }

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

    scale_resolution = GLODWORD( libvideo, SCALE_RESOLUTION );

    if ( GLOEXISTS( libvideo, SCALE_RESOLUTION_ASPECTRATIO ) ) scale_resolution_aspectratio = GLODWORD( libvideo, SCALE_RESOLUTION_ASPECTRATIO );
    if ( GLOEXISTS( libvideo, SCALE_RESOLUTION_ORIENTATION ) ) scale_resolution_orientation = GLODWORD( libvideo, SCALE_RESOLUTION_ORIENTATION );

    /* Overwrite all params */

    if ( ( e = getenv( "SCALE_RESOLUTION"             ) ) ) scale_resolution = atol( e );
    if ( ( e = getenv( "SCALE_RESOLUTION_ASPECTRATIO" ) ) ) scale_resolution_aspectratio = atol( e );
    if ( ( e = getenv( "SCALE_RESOLUTION_ORIENTATION" ) ) ) scale_resolution_orientation = atol( e );

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

    /* Inicializa el modo grafico */

    if ( scrbitmap )
    {
        bitmap_destroy( scrbitmap ) ;
        scrbitmap = NULL ;
    }

    /* Setup the SDL Window + software surfaces */

    if ( full_screen ) window_flags |= SDL_WINDOW_FULLSCREEN;
    if ( frameless ) window_flags |= SDL_WINDOW_BORDERLESS;

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

        if ( !window )
        {
            window = SDL_CreateWindow( apptitle ? apptitle : "",
                                       surface_width, surface_height, window_flags );
        }
        else
        {
            SDL_SetWindowFullscreen( window, full_screen ? true : false );
            SDL_SetWindowBordered( window, frameless ? false : true );
            SDL_SetWindowSize( window, surface_width, surface_height );
        }

        if ( !window ) return -1;

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
        if ( !window )
        {
            window = SDL_CreateWindow( apptitle ? apptitle : "",
                                       surface_width, surface_height, window_flags );
        }
        else
        {
            SDL_SetWindowFullscreen( window, full_screen ? true : false );
            SDL_SetWindowBordered( window, frameless ? false : true );
            SDL_SetWindowSize( window, surface_width, surface_height );
        }

        if ( !window ) return -1;

        screen = gr_create_shadow_surface( surface_width, surface_height, depth );
    }

    if ( !screen ) return -1;

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
    char * e;

    GLODWORD( libvideo, SCALE_RESOLUTION ) = -1; // hack for backward compatibility

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_InitSubSystem( SDL_INIT_VIDEO );

#ifdef _WIN32
    if ( !directdraw ) init_dx();
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
#ifdef _WIN32
    if ( directdraw )
    {
        /* set cooperative level back to normal */
        IDirectDraw2_SetCooperativeLevel( directdraw, NULL, DDSCL_NORMAL );

        /* release DirectDraw interface */
        IDirectDraw2_Release( directdraw );

        directdraw = NULL;
    }
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
