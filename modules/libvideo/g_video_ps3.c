/*
 * PlayStation 3 video: RSX window, software framebuffer scaled via SDL.
 * Keep the window at the TV size (720p fallback) and present the game
 * surface through the renderer. Force 32-bit so hello.prg's set_mode 16
 * does not rebuild colorghost every FRAME.
 */

#include "libvideo.h"
#include "g_video_ps3.h"

void gr_video_ps3_destroy( void )
{
}

void gr_video_ps3_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" );
    SDL_HideCursor();
}

void gr_video_ps3_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    if ( mode && mode->w > 0 && mode->h > 0 )
    {
        *width = mode->w;
        *height = mode->h;
    }
    else
    {
        *width = 1280;
        *height = 720;
    }
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_ps3_apply_mode( void )
{
    full_screen = 1;
}

int gr_video_ps3_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_ps3_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps3_present( src );
}
