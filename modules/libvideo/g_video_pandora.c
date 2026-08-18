/*
 * OpenPandora video: 800x480 LCD, software renderer over X11.
 */

#include "libvideo.h"
#include "g_video_pandora.h"

void gr_video_pandora_module_initialize( void )
{
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "software" );
    SDL_SetHint( SDL_HINT_VIDEO_DRIVER, "x11" );
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_InitSubSystem( SDL_INIT_VIDEO );
}

void gr_video_pandora_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    *width = 800;
    *height = 480;
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_pandora_apply_mode( void )
{
    full_screen = 1;
}
