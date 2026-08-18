/*
 * Nintendo Switch video: 720p/1080p GLES window, software framebuffer presented scaled.
 */

#include "libvideo.h"
#include "g_video_switch.h"

void gr_video_switch_module_initialize( void )
{
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "opengles2" );
}

void gr_video_switch_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    /* mesa fatalThrows if the NWindow is not 720p/1080p. Keep the game
     * framebuffer at the requested size and present it scaled. */
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
    *window_flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL;
}

void gr_video_switch_apply_mode( void )
{
    full_screen = 1;
}

int gr_video_switch_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_switch_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_switch_present( src );
}
