/*
 * Apple tvOS video: 1920x1080 (or the display mode) Metal window, software
 * framebuffer presented scaled. Keep the game buffer at set_mode size.
 */

#include "libvideo.h"
#include "g_video_tvos.h"

void gr_video_tvos_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "metal" );
}

void gr_video_tvos_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    if ( mode && mode->w > 0 && mode->h > 0 )
    {
        *width = mode->w;
        *height = mode->h;
    }
    else
    {
        *width = 1920;
        *height = 1080;
    }
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_tvos_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

int gr_video_tvos_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_tvos_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_tvos_present( src );
}
