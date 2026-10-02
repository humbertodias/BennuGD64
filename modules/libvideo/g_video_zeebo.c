/*
 * Zeebo video: VGA 640x480, software renderer. BREW presents this later;
 * the ELF itself uses SDL's dummy/offscreen driver.
 */

#include "libvideo.h"
#include "g_video_zeebo.h"

void gr_video_zeebo_module_initialize( void )
{
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "software" );
    SDL_SetHint( SDL_HINT_VIDEO_DRIVER, "dummy" );
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_InitSubSystem( SDL_INIT_VIDEO );
}

void gr_video_zeebo_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    *width = 640;
    *height = 480;
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_zeebo_apply_mode( void )
{
    full_screen = 1;
}
