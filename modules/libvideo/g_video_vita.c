/*
 * PlayStation Vita video: 960x544 GXM window, software framebuffer presented scaled.
 *
 * GXM has no XRGB8888. 32-bit Bennu is 0x00RRGGBB (ARGB8888 on LE); those
 * textures default to blend and 0x00 alpha made the screen black. 16-bit
 * SoRR stays RGB565 so software blit is half the bandwidth of a forced
 * 32-bit buffer, and GXM uploads it without ConvertSurface every FRAME.
 */

#include "libvideo.h"
#include "g_video_vita.h"

static SDL_Renderer * vita_renderer = NULL ;
static SDL_Texture * vita_texture = NULL ;
static SDL_PixelFormat vita_tex_fmt = SDL_PIXELFORMAT_UNKNOWN ;
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
    vita_tex_fmt = SDL_PIXELFORMAT_UNKNOWN;
}

void gr_video_vita_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "VITA gxm" );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "0" );
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
    /* Stretch the game buffer on the GPU. CPU 2x / SCALE_RESOLUTION to
     * 960x544 would blit a full HD surface every FRAME on a 444 MHz ARM. */
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

static SDL_PixelFormat vita_present_format( SDL_Surface * src )
{
    if ( src && bennu_surface_bpp( src ) == 16 )
        return SDL_PIXELFORMAT_RGB565;
    return SDL_PIXELFORMAT_ARGB8888;
}

int gr_video_vita_present( SDL_Surface * src )
{
    SDL_PixelFormat fmt ;

    if ( !window || !src || !src->pixels ) return 0;

    if ( !vita_renderer )
    {
        vita_renderer = SDL_CreateRenderer( window, "VITA gxm" );
        if ( !vita_renderer )
            vita_renderer = SDL_CreateRenderer( window, NULL );
        if ( !vita_renderer ) return 0;
        SDL_SetRenderVSync( vita_renderer, 0 );
    }

    fmt = vita_present_format( src );

    if ( !vita_texture || vita_tex_w != src->w || vita_tex_h != src->h || vita_tex_fmt != fmt )
    {
        if ( vita_texture ) SDL_DestroyTexture( vita_texture );
        vita_texture = SDL_CreateTexture( vita_renderer, fmt,
                                          SDL_TEXTUREACCESS_STREAMING, src->w, src->h );
        if ( !vita_texture ) return 0;
        vita_tex_w = src->w;
        vita_tex_h = src->h;
        vita_tex_fmt = fmt;
        SDL_SetTextureScaleMode( vita_texture, SDL_SCALEMODE_NEAREST );
        SDL_SetTextureBlendMode( vita_texture, SDL_BLENDMODE_NONE );
    }

    if ( ( fmt == SDL_PIXELFORMAT_RGB565 && bennu_surface_bpp( src ) == 16 ) ||
         ( fmt == SDL_PIXELFORMAT_ARGB8888 && bennu_surface_bytes_pp( src ) == 4 ) )
    {
        if ( !SDL_UpdateTexture( vita_texture, NULL, src->pixels, src->pitch ) )
            return 0;
    }
    else
    {
        SDL_Surface * converted = SDL_ConvertSurface( src, fmt );
        if ( !converted ) return 0;
        if ( !SDL_UpdateTexture( vita_texture, NULL, converted->pixels, converted->pitch ) )
        {
            SDL_DestroySurface( converted );
            return 0;
        }
        SDL_DestroySurface( converted );
    }

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
