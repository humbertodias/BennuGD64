/*
 * Nintendo Wii present path: GX renderer ("OGC EFB"). Window-surface
 * blits go through a CPU→tiled-texture convert that DSIs in Dolphin.
 * Keep the window at the TV size and scale the game framebuffer; exclusive
 * fullscreen vsync-hangs in this SDL3-libogc2 build.
 */

#include "libvideo.h"
#include "g_video_wii.h"

void gr_video_wii_destroy( void )
{
}

void gr_video_wii_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "OGC EFB" );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "0" );
    SDL_HideCursor();
}

void gr_video_wii_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    if ( mode && mode->w > 0 && mode->h > 0 )
    {
        *width = mode->w;
        *height = mode->h;
    }
    else
    {
        *width = 640;
        *height = 480;
    }
    *window_flags &= ~SDL_WINDOW_FULLSCREEN ;
}

void gr_video_wii_apply_mode( void )
{
    full_screen = 0 ;
}

int gr_video_wii_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_wii_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects ;
    ( void ) count ;
    return gr_video_wii_present( src );
}
