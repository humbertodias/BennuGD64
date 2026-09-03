/*
 * PlayStation 4 video: fullscreen window, software framebuffer presented scaled.
 *
 * Needs an SDL3 video backend that can create a window/renderer on Orbis
 * (official SDL3 PS4 is NDA-only; community/OpenOrbis SDL2 is separate).
 */

#include "libvideo.h"
#include "g_video_ps4.h"

static SDL_Renderer * ps4_renderer = NULL ;
static SDL_Texture * ps4_texture = NULL ;
static SDL_PixelFormat ps4_tex_fmt = SDL_PIXELFORMAT_UNKNOWN ;
static int ps4_tex_w = 0 ;
static int ps4_tex_h = 0 ;

void gr_video_ps4_destroy( void )
{
    if ( ps4_texture )
    {
        SDL_DestroyTexture( ps4_texture );
        ps4_texture = NULL;
    }
    if ( ps4_renderer )
    {
        SDL_DestroyRenderer( ps4_renderer );
        ps4_renderer = NULL;
    }
    ps4_tex_w = 0;
    ps4_tex_h = 0;
    ps4_tex_fmt = SDL_PIXELFORMAT_UNKNOWN;
}

void gr_video_ps4_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "0" );
    SDL_HideCursor();
}

void gr_video_ps4_adjust_window( int * width, int * height, Uint32 * window_flags )
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

void gr_video_ps4_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

static SDL_PixelFormat ps4_present_format( SDL_Surface * src )
{
    if ( src && bennu_surface_bpp( src ) == 16 )
        return SDL_PIXELFORMAT_RGB565;
    return SDL_PIXELFORMAT_ARGB8888;
}

int gr_video_ps4_present( SDL_Surface * src )
{
    SDL_PixelFormat fmt ;

    if ( !window || !src || !src->pixels ) return 0;

    if ( !ps4_renderer )
    {
        ps4_renderer = SDL_CreateRenderer( window, NULL );
        if ( !ps4_renderer ) return 0;
        SDL_SetRenderVSync( ps4_renderer, 0 );
    }

    fmt = ps4_present_format( src );

    if ( !ps4_texture || ps4_tex_w != src->w || ps4_tex_h != src->h || ps4_tex_fmt != fmt )
    {
        if ( ps4_texture ) SDL_DestroyTexture( ps4_texture );
        ps4_texture = SDL_CreateTexture( ps4_renderer, fmt,
                                          SDL_TEXTUREACCESS_STREAMING, src->w, src->h );
        if ( !ps4_texture ) return 0;
        ps4_tex_w = src->w;
        ps4_tex_h = src->h;
        ps4_tex_fmt = fmt;
        SDL_SetTextureScaleMode( ps4_texture, SDL_SCALEMODE_NEAREST );
        SDL_SetTextureBlendMode( ps4_texture, SDL_BLENDMODE_NONE );
    }

    if ( src->format == fmt )
    {
        if ( !SDL_UpdateTexture( ps4_texture, NULL, src->pixels, src->pitch ) )
            return 0;
    }
    else
    {
        SDL_Surface * converted = SDL_ConvertSurface( src, fmt );
        if ( !converted ) return 0;
        if ( !SDL_UpdateTexture( ps4_texture, NULL, converted->pixels, converted->pitch ) )
        {
            SDL_DestroySurface( converted );
            return 0;
        }
        SDL_DestroySurface( converted );
    }

    SDL_SetRenderDrawColor( ps4_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( ps4_renderer );
    SDL_RenderTexture( ps4_renderer, ps4_texture, NULL, NULL );
    SDL_RenderPresent( ps4_renderer );
    return 1;
}

int gr_video_ps4_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps4_present( src );
}
