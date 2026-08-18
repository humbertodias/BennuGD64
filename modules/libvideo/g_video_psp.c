/*
 * PlayStation Portable present path: software RGB565 framebuffer uploaded
 * as BGR565 tiles through the SDL GU renderer (max texture size 512 px).
 */

#include <stdint.h>

#include "libvideo.h"
#include "g_video_psp.h"

/* SDL PSP GU renderer rejects streaming textures wider or taller than 512 px. */
#define PSP_MAX_TEXTURE_SIZE 512

static SDL_Renderer * present_renderer = NULL ;
static SDL_Texture * present_texture = NULL ;
static int present_tex_w = 0 ;
static int present_tex_h = 0 ;
static SDL_PixelFormat present_tex_fmt = SDL_PIXELFORMAT_UNKNOWN ;
static int present_logical_w = 0 ;
static int present_logical_h = 0 ;

void gr_video_psp_destroy( void )
{
    if ( present_texture )
    {
        SDL_DestroyTexture( present_texture );
        present_texture = NULL;
    }
    if ( present_renderer )
    {
        SDL_DestroyRenderer( present_renderer );
        present_renderer = NULL;
    }
    present_tex_w = 0;
    present_tex_h = 0;
    present_tex_fmt = SDL_PIXELFORMAT_UNKNOWN;
    present_logical_w = 0;
    present_logical_h = 0;
}

void gr_video_psp_module_initialize( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "psp" );
    SDL_SetHint( SDL_HINT_RENDER_VSYNC, "0" );
}

void gr_video_psp_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    if ( mode && mode->w > 0 && mode->h > 0 )
    {
        *width = mode->w;
        *height = mode->h;
    }
    else
    {
        *width = 480;
        *height = 272;
    }
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_psp_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
}

static int gr_psp_init_present_renderer( int logical_w, int logical_h )
{
    if ( !window || logical_w < 1 || logical_h < 1 ) return 0 ;

    if ( present_renderer &&
         present_logical_w == logical_w && present_logical_h == logical_h )
        return 1 ;

    gr_video_psp_destroy();
    present_renderer = SDL_CreateRenderer( window, "psp" );
    if ( !present_renderer )
        present_renderer = SDL_CreateRenderer( window, NULL );
    if ( !present_renderer ) return 0 ;

    SDL_SetRenderVSync( present_renderer, false );
    if ( !SDL_SetRenderLogicalPresentation( present_renderer, logical_w, logical_h,
                                            SDL_LOGICAL_PRESENTATION_STRETCH ) )
    {
        gr_video_psp_destroy();
        return 0 ;
    }

    present_logical_w = logical_w ;
    present_logical_h = logical_h ;
    return 1 ;
}

int gr_video_psp_ready_present( int logical_w, int logical_h )
{
    return gr_psp_init_present_renderer( logical_w, logical_h );
}

static SDL_PixelFormat gr_psp_present_format( SDL_Surface * src )
{
    /* Game framebuffers stay RGB565 (Bennu convention). The GU renderer
     * uploads BGR565, so present converts at upload time. */
    if ( src && bennu_surface_bpp( src ) == 16 )
        return SDL_PIXELFORMAT_BGR565;
    return SDL_PIXELFORMAT_ABGR8888;
}

static void gr_psp_convert_rgb565_row( const uint16_t * src, uint16_t * dst, int count )
{
    int i = 0;

    if ( count >= 2 && !( ( ( uintptr_t ) src | ( uintptr_t ) dst ) & 3 ) )
    {
        while ( i + 3 < count )
        {
            uint32_t p0 = *( const uint32_t * )( src + i );
            uint32_t p1 = *( const uint32_t * )( src + i + 2 );
            uint32_t g0 = p0 & 0x07E007E0u;
            uint32_t g1 = p1 & 0x07E007E0u;
            uint32_t rb0 = p0 & 0xF81FF81Fu;
            uint32_t rb1 = p1 & 0xF81FF81Fu;

            *( uint32_t * )( dst + i ) = g0 | ( ( rb0 & 0x001F001Fu ) << 11 ) | ( ( rb0 & 0xF800F800u ) >> 11 );
            *( uint32_t * )( dst + i + 2 ) = g1 | ( ( rb1 & 0x001F001Fu ) << 11 ) | ( ( rb1 & 0xF800F800u ) >> 11 );
            i += 4;
        }
        while ( i + 1 < count )
        {
            uint32_t p = *( const uint32_t * )( src + i );
            uint32_t g = p & 0x07E007E0u;
            uint32_t rb = p & 0xF81FF81Fu;

            *( uint32_t * )( dst + i ) = g | ( ( rb & 0x001F001Fu ) << 11 ) | ( ( rb & 0xF800F800u ) >> 11 );
            i += 2;
        }
    }

    while ( i < count )
    {
        uint16_t c = src[ i ];
        dst[ i ] = ( uint16_t )( ( c & 0x07E0 ) | ( ( c & 0x001F ) << 11 ) | ( ( c & 0xF800 ) >> 11 ) );
        i++ ;
    }
}

static int gr_psp_ensure_tile_texture( int tw, int th, SDL_PixelFormat tex_fmt )
{
    int need_w, need_h ;

    if ( !present_renderer || tw < 1 || th < 1 ) return 0;

    need_w = tw > PSP_MAX_TEXTURE_SIZE ? PSP_MAX_TEXTURE_SIZE : tw;
    need_h = th > PSP_MAX_TEXTURE_SIZE ? PSP_MAX_TEXTURE_SIZE : th;

    /* Keep a texture at least as large as this upload so we never recreate
     * between 512-wide and leftover tiles. */
    if ( present_texture && present_tex_fmt == tex_fmt &&
         present_tex_w >= need_w && present_tex_h >= need_h )
        return 1;

    if ( present_texture ) SDL_DestroyTexture( present_texture );
    present_texture = SDL_CreateTexture( present_renderer, tex_fmt,
                                         SDL_TEXTUREACCESS_STREAMING, need_w, need_h );
    if ( !present_texture ) return 0;
    present_tex_w = need_w;
    present_tex_h = need_h;
    present_tex_fmt = tex_fmt;
    SDL_SetTextureScaleMode( present_texture, SDL_SCALEMODE_NEAREST );
    return 1;
}

static int gr_psp_blit_rect_to_texture( SDL_Surface * src, int sx, int sy, int sw, int sh,
                                        int dx, int dy, SDL_PixelFormat tex_fmt )
{
    SDL_Rect lock_rect ;
    int bpp = bennu_surface_bytes_pp( src );
    int row ;

    lock_rect.x = dx;
    lock_rect.y = dy;
    lock_rect.w = sw;
    lock_rect.h = sh;

    if ( tex_fmt == SDL_PIXELFORMAT_BGR565 && bpp == 2 )
    {
        void * locked ;
        int locked_pitch ;

        if ( !SDL_LockTexture( present_texture, &lock_rect, &locked, &locked_pitch ) )
            return 0;

        for ( row = 0 ; row < sh ; row++ )
        {
            const uint16_t * srow = ( const uint16_t * )( ( const Uint8 * ) src->pixels +
                                                         ( sy + row ) * src->pitch + sx * 2 );
            uint16_t * drow = ( uint16_t * )( ( Uint8 * ) locked + row * locked_pitch );

            gr_psp_convert_rgb565_row( srow, drow, sw );
        }

        SDL_UnlockTexture( present_texture );
        return 1;
    }

    {
        const void * pixels = ( const Uint8 * ) src->pixels + sy * src->pitch + sx * bpp;
        return SDL_UpdateTexture( present_texture, &lock_rect, pixels, src->pitch ) ? 1 : 0;
    }
}

static int gr_psp_draw_tiles( SDL_Surface * src, SDL_PixelFormat tex_fmt )
{
    int tx, ty, tw, th ;

    for ( ty = 0 ; ty < src->h ; ty += PSP_MAX_TEXTURE_SIZE )
    {
        th = src->h - ty;
        if ( th > PSP_MAX_TEXTURE_SIZE ) th = PSP_MAX_TEXTURE_SIZE;

        for ( tx = 0 ; tx < src->w ; tx += PSP_MAX_TEXTURE_SIZE )
        {
            SDL_FRect src_f, dst ;

            tw = src->w - tx;
            if ( tw > PSP_MAX_TEXTURE_SIZE ) tw = PSP_MAX_TEXTURE_SIZE;

            if ( !gr_psp_ensure_tile_texture( tw, th, tex_fmt ) ) return 0;
            if ( !gr_psp_blit_rect_to_texture( src, tx, ty, tw, th, 0, 0, tex_fmt ) ) return 0;

            src_f.x = 0.0f;
            src_f.y = 0.0f;
            src_f.w = ( float ) tw;
            src_f.h = ( float ) th;
            dst.x = ( float ) tx;
            dst.y = ( float ) ty;
            dst.w = ( float ) tw;
            dst.h = ( float ) th;
            SDL_RenderTexture( present_renderer, present_texture, &src_f, &dst );
        }
    }

    return 1;
}

int gr_video_psp_present( SDL_Surface * src )
{
    SDL_PixelFormat tex_fmt ;

    if ( !window || !src || !src->pixels ) return 0 ;
    if ( !gr_psp_init_present_renderer( src->w, src->h ) ) return 0 ;

    tex_fmt = gr_psp_present_format( src );

    if ( !gr_psp_draw_tiles( src, tex_fmt ) )
        return 0;

    SDL_RenderPresent( present_renderer );
    return 1;
}

int gr_video_psp_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    SDL_PixelFormat tex_fmt ;
    int i, total_area = 0 ;
    const int frame_area = src->w * src->h ;

    if ( !window || !src || !src->pixels || count <= 0 ) return 0 ;
    if ( !gr_psp_init_present_renderer( src->w, src->h ) ) return 0 ;

    tex_fmt = gr_psp_present_format( src );

    for ( i = 0 ; i < count ; i++ )
        total_area += rects[ i ].w * rects[ i ].h;

    /* Scrolling stages dirty almost the whole screen; skip the extra pass. */
    if ( total_area >= frame_area * 2 / 3 )
        return gr_video_psp_present( src );

    /* One texture the size of the game buffer: convert only dirty pixels,
     * then stretch the whole texture once. */
    if ( src->w <= PSP_MAX_TEXTURE_SIZE && src->h <= PSP_MAX_TEXTURE_SIZE )
    {
        if ( !gr_psp_ensure_tile_texture( src->w, src->h, tex_fmt ) ) return 0;

        for ( i = 0 ; i < count ; i++ )
        {
            int x = rects[ i ].x, y = rects[ i ].y, w = rects[ i ].w, h = rects[ i ].h ;

            if ( x < 0 ) { w += x; x = 0; }
            if ( y < 0 ) { h += y; y = 0; }
            if ( x + w > src->w ) w = src->w - x;
            if ( y + h > src->h ) h = src->h - y;
            if ( w <= 0 || h <= 0 ) continue;

            if ( !gr_psp_blit_rect_to_texture( src, x, y, w, h, x, y, tex_fmt ) )
                return 0;
        }

        SDL_RenderTexture( present_renderer, present_texture, NULL, NULL );
        SDL_RenderPresent( present_renderer );
        return 1;
    }

    return gr_video_psp_present( src );
}
