/*
 * Dreamcast video: DIRECT_VIDEO window surface, 1:1 blit (no software scale on SH4).
 */

#include "libvideo.h"
#include "g_video_dc.h"

void SDL_DC_ShowAskHz( bool value );
void SDL_DC_Default60Hz( bool value );

void gr_video_dc_module_initialize( void )
{
    SDL_SetHint( SDL_HINT_DC_VIDEO_MODE, "SDL_DC_DIRECT_VIDEO" );
    /* CreateWindowFramebuffer only allocates a backbuffer when this hint is
     * set; UpdateWindowFramebuffer still defaults it to true. Unset, the first
     * blit hits VRAM and the next present copies from a NULL buffer (flash
     * then black). GPF samples always set both hints together. */
    SDL_SetHint( SDL_HINT_VIDEO_DOUBLE_BUFFER, "1" );
    SDL_DC_ShowAskHz( false );
    SDL_DC_Default60Hz( true );
}

void gr_video_dc_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    ( void ) width;
    ( void ) height;
    /* DIRECT_VIDEO supports 320x240 and 640x480 natively. */
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_dc_apply_mode( void )
{
    full_screen = 1;
}

int gr_video_dc_present( SDL_Surface * src )
{
    SDL_Surface * winsurf ;

    if ( !window || !src ) return 0 ;

    winsurf = SDL_GetWindowSurface( window );
    if ( !winsurf ) return 0;

    if ( winsurf->w == src->w && winsurf->h == src->h )
        SDL_BlitSurface( src, NULL, winsurf, NULL );
    else
        SDL_BlitSurfaceScaled( src, NULL, winsurf, NULL, SDL_SCALEMODE_NEAREST );

    SDL_UpdateWindowSurface( window );
    return 1;
}
