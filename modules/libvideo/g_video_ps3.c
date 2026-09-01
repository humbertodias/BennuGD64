/*
 * PlayStation 3 video: 720p RSX window, software framebuffer presented scaled.
 *
 * RSX textures are A8R8G8B8 / R5G6B5. XRGB8888 is not native; 32-bit Bennu
 * is 0x00RRGGBB (alpha 0) and default blend made the screen black. SoRR is
 * 16-bit RGB565 — upload that without ConvertSurface. Do not force 32-bit.
 */

#include "libvideo.h"
#include "g_video_ps3.h"

static SDL_Renderer * ps3_renderer = NULL ;
static SDL_Texture * ps3_texture = NULL ;
static SDL_PixelFormat ps3_tex_fmt = SDL_PIXELFORMAT_UNKNOWN ;
static int ps3_tex_w = 0 ;
static int ps3_tex_h = 0 ;

void gr_video_ps3_destroy( void )
{
    if ( ps3_texture )
    {
        SDL_DestroyTexture( ps3_texture );
        ps3_texture = NULL;
    }
    if ( ps3_renderer )
    {
        SDL_DestroyRenderer( ps3_renderer );
        ps3_renderer = NULL;
    }
    ps3_tex_w = 0;
    ps3_tex_h = 0;
    ps3_tex_fmt = SDL_PIXELFORMAT_UNKNOWN;
}

void gr_video_ps3_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "0" );
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
    /* Stretch the game buffer on the GPU. CPU 2x / SCALE_RESOLUTION to
     * 720p would blit a full HD surface every FRAME. */
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

static SDL_PixelFormat ps3_present_format( SDL_Surface * src )
{
    if ( src && bennu_surface_bpp( src ) == 16 )
        return SDL_PIXELFORMAT_RGB565;
    return SDL_PIXELFORMAT_ARGB8888;
}

int gr_video_ps3_present( SDL_Surface * src )
{
    SDL_PixelFormat fmt ;

    if ( !window || !src || !src->pixels ) return 0;

    if ( !ps3_renderer )
    {
        ps3_renderer = SDL_CreateRenderer( window, NULL );
        if ( !ps3_renderer ) return 0;
        SDL_SetRenderVSync( ps3_renderer, 0 );
    }

    fmt = ps3_present_format( src );

    if ( !ps3_texture || ps3_tex_w != src->w || ps3_tex_h != src->h || ps3_tex_fmt != fmt )
    {
        if ( ps3_texture ) SDL_DestroyTexture( ps3_texture );
        ps3_texture = SDL_CreateTexture( ps3_renderer, fmt,
                                          SDL_TEXTUREACCESS_STREAMING, src->w, src->h );
        if ( !ps3_texture ) return 0;
        ps3_tex_w = src->w;
        ps3_tex_h = src->h;
        ps3_tex_fmt = fmt;
        SDL_SetTextureScaleMode( ps3_texture, SDL_SCALEMODE_NEAREST );
        SDL_SetTextureBlendMode( ps3_texture, SDL_BLENDMODE_NONE );
    }

    if ( src->format == fmt )
    {
        if ( !SDL_UpdateTexture( ps3_texture, NULL, src->pixels, src->pitch ) )
            return 0;
    }
    else
    {
        SDL_Surface * converted = SDL_ConvertSurface( src, fmt );
        if ( !converted ) return 0;
        if ( !SDL_UpdateTexture( ps3_texture, NULL, converted->pixels, converted->pitch ) )
        {
            SDL_DestroySurface( converted );
            return 0;
        }
        SDL_DestroySurface( converted );
    }

    SDL_SetRenderDrawColor( ps3_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( ps3_renderer );
    SDL_RenderTexture( ps3_renderer, ps3_texture, NULL, NULL );
    SDL_RenderPresent( ps3_renderer );
    return 1;
}

int gr_video_ps3_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps3_present( src );
}
