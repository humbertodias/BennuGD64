/*
 * PlayStation Vita video: 960x544 GXM window, software framebuffer presented scaled.
 */

#include "libvideo.h"
#include "g_video_vita.h"

void gr_video_vita_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" );
}

void gr_video_vita_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    if ( mode && mode->w > 0 && mode->h > 0 )
    {
        *width = mode->w;
        *height = mode->h;
    }
    else
    {
        *width = 960;
        *height = 544;
    }
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_vita_apply_mode( void )
{
    /* SoRR-vita hardcoded the GXM window to 960x544 fullscreen; depth is
     * forced to 32 in gr_set_mode() like the PS2 16-bit override. */
    full_screen = 1;
}

int gr_video_vita_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_vita_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_vita_present( src );
}
