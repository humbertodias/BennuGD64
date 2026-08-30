/*
 * PlayStation Vita video: 960x544 GXM window, software framebuffer presented scaled.
 *
 * GXM has no XRGB8888. Uploading that format is treated as ABGR and swaps
 * R/B (SoRR Bomber Games logo: blue → red). Bennu 32-bit is 0x00RRGGBB,
 * which is ARGB8888 on little-endian. ARGB textures default to blend, and
 * the 0x00 alpha makes every pixel invisible (black screen except maps
 * that wrote a non-zero high byte). Disable blending so RGB is shown.
 */

#include "libvideo.h"
#include "g_video_vita.h"

static SDL_Renderer * vita_renderer = NULL ;
static SDL_Texture * vita_texture = NULL ;
static int vita_tex_w = 0 ;
static int vita_tex_h = 0 ;

void gr_video_vita_destroy( void )
{
    if ( vita_texture )
    {
        SDL_DestroyTexture( vita_texture );
        vita_texture = NULL;
    }
    if ( vita_renderer )
    {
        SDL_DestroyRenderer( vita_renderer );
        vita_renderer = NULL;
    }
    vita_tex_w = 0;
    vita_tex_h = 0;
}

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
    if ( !window || !src ) return 0;

    if ( !vita_renderer )
    {
        vita_renderer = SDL_CreateRenderer( window, NULL );
        if ( !vita_renderer ) return 0;
        SDL_SetRenderVSync( vita_renderer, false );
    }

    if ( !vita_texture || vita_tex_w != src->w || vita_tex_h != src->h )
    {
        if ( vita_texture ) SDL_DestroyTexture( vita_texture );
        vita_texture = SDL_CreateTexture( vita_renderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING, src->w, src->h );
        if ( !vita_texture ) return 0;
        vita_tex_w = src->w;
        vita_tex_h = src->h;
        SDL_SetTextureScaleMode( vita_texture, SDL_SCALEMODE_NEAREST );
        SDL_SetTextureBlendMode( vita_texture, SDL_BLENDMODE_NONE );
    }

    if ( bennu_surface_bytes_pp( src ) == 4 )
    {
        if ( !SDL_UpdateTexture( vita_texture, NULL, src->pixels, src->pitch ) )
            return 0;
    }
    else
    {
        SDL_Surface * converted = SDL_ConvertSurface( src, SDL_PIXELFORMAT_ARGB8888 );
        if ( !converted ) return 0;
        if ( !SDL_UpdateTexture( vita_texture, NULL, converted->pixels, converted->pitch ) )
        {
            SDL_DestroySurface( converted );
            return 0;
        }
        SDL_DestroySurface( converted );
    }

    SDL_SetRenderDrawColor( vita_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( vita_renderer );
    SDL_RenderTexture( vita_renderer, vita_texture, NULL, NULL );
    SDL_RenderPresent( vita_renderer );
    return 1;
}

int gr_video_vita_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_vita_present( src );
}
