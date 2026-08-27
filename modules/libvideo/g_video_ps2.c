/*
 * PlayStation 2 video: gsKit renderer. Window-surface blits are not used.
 */

#include "libvideo.h"
#include "g_video_ps2.h"

#ifndef SDL_HINT_PS2_GS_WIDTH
#define SDL_HINT_PS2_GS_WIDTH "SDL_PS2_GS_WIDTH"
#endif
#ifndef SDL_HINT_PS2_GS_HEIGHT
#define SDL_HINT_PS2_GS_HEIGHT "SDL_PS2_GS_HEIGHT"
#endif

void gr_video_ps2_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "PS2 gsKit" );
    SDL_SetHint( SDL_HINT_PS2_GS_WIDTH, "640" );
    SDL_SetHint( SDL_HINT_PS2_GS_HEIGHT, "448" );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" );
    SDL_HideCursor();
}

void gr_video_ps2_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    *width = 640;
    *height = 448;
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_ps2_apply_mode( void )
{
    full_screen = 1;
}

int gr_video_ps2_present( SDL_Surface * src )
{
    return gr_video_present_via_renderer( src );
}

int gr_video_ps2_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps2_present( src );
}
