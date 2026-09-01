/*
 * Apple iOS video: landscape Metal window at the display size, software
 * framebuffer presented scaled. Keep the game buffer at set_mode size.
 */

#include "libvideo.h"
#include "g_video_ios.h"

void gr_video_ios_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "metal" );
    SDL_SetHint( SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight" );
}

void gr_video_ios_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );
    int w, h;

    if ( mode && mode->w > 0 && mode->h > 0 )
    {
        w = mode->w;
        h = mode->h;
    }
    else
    {
        w = 1920;
        h = 1080;
    }
    if ( h > w )
    {
        int tmp = w;
        w = h;
        h = tmp;
    }
    *width = w;
    *height = h;
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_ios_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

int gr_video_ios_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_ios_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ios_present( src );
}
