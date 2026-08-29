/*
 * PlayStation 2 video: gsKit renderer. Window-surface blits are not used.
 *
 * Game buffer is RGB565. gsKit textures are ABGR1555. Convert in LockTexture
 * (no per-frame malloc). Dirty rects skip a full-frame convert when small.
 * GS stays 640x448 (NTSC). 480 hangs gsKit/PCSX2 on the FREEWARE present.
 */

#include <stdint.h>
#include <string.h>

#include "libvideo.h"
#include "g_video_ps2.h"

#ifndef SDL_HINT_PS2_GS_WIDTH
#define SDL_HINT_PS2_GS_WIDTH "SDL_PS2_GS_WIDTH"
#endif
#ifndef SDL_HINT_PS2_GS_HEIGHT
#define SDL_HINT_PS2_GS_HEIGHT "SDL_PS2_GS_HEIGHT"
#endif

static SDL_Renderer * ps2_renderer = NULL;
static SDL_Texture * ps2_texture = NULL;
static int ps2_tex_w = 0;
static int ps2_tex_h = 0;

void gr_video_ps2_module_initialize( void )
{
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
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
    scale_resolution = -1;

    if ( !SDL_WasInit( SDL_INIT_EVENTS ) )
        SDL_InitSubSystem( SDL_INIT_EVENTS );
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO );
}

void gr_video_ps2_before_window( void )
{
}

void gr_video_ps2_after_window( void )
{
}

static void rgb565_to_abgr1555( const uint16_t * src, uint16_t * dst, int count )
{
    int i = 0;

    if ( count >= 2 && !( ( ( uintptr_t ) src | ( uintptr_t ) dst ) & 3 ) )
    {
        while ( i + 1 < count )
        {
            uint32_t p = *( const uint32_t * )( src + i );
            uint32_t c0 = p & 0xFFFFu;
            uint32_t c1 = p >> 16;
            uint32_t o0 = 0x8000u | ( ( c0 & 0x001F ) << 10 ) | ( ( c0 & 0x07C0 ) >> 1 ) | ( c0 >> 11 );
            uint32_t o1 = 0x8000u | ( ( c1 & 0x001F ) << 10 ) | ( ( c1 & 0x07C0 ) >> 1 ) | ( c1 >> 11 );
            *( uint32_t * )( dst + i ) = o0 | ( o1 << 16 );
            i += 2;
        }
    }
    while ( i < count )
    {
        uint32_t c = src[ i ];
        dst[ i ] = ( uint16_t )( 0x8000u | ( ( c & 0x001F ) << 10 ) | ( ( c & 0x07C0 ) >> 1 ) | ( c >> 11 ) );
        i++;
    }
}

static int ps2_ensure_renderer( int w, int h )
{
    if ( !window || w < 1 || h < 1 )
        return 0;

    if ( !ps2_renderer )
    {
        ps2_renderer = SDL_CreateRenderer( window, "PS2 gsKit" );
        if ( !ps2_renderer )
            ps2_renderer = SDL_CreateRenderer( window, NULL );
        if ( !ps2_renderer )
            return 0;
        SDL_SetRenderVSync( ps2_renderer, false );
    }

    if ( !ps2_texture || ps2_tex_w != w || ps2_tex_h != h )
    {
        if ( ps2_texture )
            SDL_DestroyTexture( ps2_texture );
        ps2_texture = SDL_CreateTexture( ps2_renderer, SDL_PIXELFORMAT_ABGR1555,
                                         SDL_TEXTUREACCESS_STREAMING, w, h );
        if ( !ps2_texture )
            return 0;
        ps2_tex_w = w;
        ps2_tex_h = h;
        SDL_SetTextureScaleMode( ps2_texture, SDL_SCALEMODE_NEAREST );
    }
    return 1;
}

static int ps2_upload_rect( SDL_Surface * src, int x, int y, int w, int h )
{
    SDL_Rect lock;
    void * pixels;
    int pitch, row;
    int bpp = bennu_surface_bytes_pp( src );

    if ( x < 0 ) { w += x; x = 0; }
    if ( y < 0 ) { h += y; y = 0; }
    if ( x + w > src->w ) w = src->w - x;
    if ( y + h > src->h ) h = src->h - y;
    if ( w <= 0 || h <= 0 )
        return 1;

    lock.x = x;
    lock.y = y;
    lock.w = w;
    lock.h = h;
    if ( !SDL_LockTexture( ps2_texture, &lock, &pixels, &pitch ) )
        return 0;

    if ( bpp == 2 )
    {
        for ( row = 0; row < h; row++ )
        {
            const uint16_t * srow = ( const uint16_t * )( ( const Uint8 * ) src->pixels +
                                                          ( y + row ) * src->pitch + x * 2 );
            uint16_t * drow = ( uint16_t * )( ( Uint8 * ) pixels + row * pitch );
            rgb565_to_abgr1555( srow, drow, w );
        }
    }
    else
    {
        const Uint8 * srow = ( const Uint8 * ) src->pixels + y * src->pitch + x * bpp;
        Uint8 * drow = ( Uint8 * ) pixels;
        int copy = w * bpp;
        for ( row = 0; row < h; row++ )
        {
            memcpy( drow, srow, ( size_t ) copy );
            srow += src->pitch;
            drow += pitch;
        }
    }

    SDL_UnlockTexture( ps2_texture );
    return 1;
}

static int ps2_flip( void )
{
    if ( !SDL_RenderTexture( ps2_renderer, ps2_texture, NULL, NULL ) )
        return 0;
    return SDL_RenderPresent( ps2_renderer ) ? 1 : 0;
}

int gr_video_ps2_present( SDL_Surface * src )
{
    if ( !src || !src->pixels )
        return 0;
    if ( !ps2_ensure_renderer( src->w, src->h ) )
        return 0;
    if ( !ps2_upload_rect( src, 0, 0, src->w, src->h ) )
        return 0;
    return ps2_flip();
}

int gr_video_ps2_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    int i, area = 0;
    int frame;

    if ( !src || !src->pixels || !rects || count <= 0 )
        return 0;
    if ( !ps2_ensure_renderer( src->w, src->h ) )
        return 0;

    frame = src->w * src->h;
    for ( i = 0; i < count; i++ )
        area += rects[ i ].w * rects[ i ].h;
    if ( area >= frame * 2 / 3 )
        return gr_video_ps2_present( src );

    for ( i = 0; i < count; i++ )
    {
        if ( !ps2_upload_rect( src, rects[ i ].x, rects[ i ].y, rects[ i ].w, rects[ i ].h ) )
            return 0;
    }
    return ps2_flip();
}
