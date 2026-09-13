/* nxdk framebuffer presenter for Bennu's software surface. */

#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <hal/video.h>

#include "libvideo.h"
#include "sdl3_compat.h"
#include "g_video_xbox.h"

static unsigned char * xbox_fb;
static int xbox_width;
static int xbox_height;
static int xbox_pitch;
static int xbox_bpp;
static int xbox_source_width;
static int xbox_source_height;
static int * xbox_scale_x;
static int * xbox_scale_y;
static uint32_t xbox_rgb565[65536];
static int xbox_rgb565_ready;

static uint32_t xbox_from_565( uint16_t pixel )
{
    uint32_t r = ( pixel >> 11 ) & 31u;
    uint32_t g = ( pixel >> 5 ) & 63u;
    uint32_t b = pixel & 31u;
    r = ( r << 3 ) | ( r >> 2 );
    g = ( g << 2 ) | ( g >> 4 );
    b = ( b << 3 ) | ( b >> 2 );
    /* nxdk 32bpp FB is D3DFMT_A8R8G8B8 (0xAARRGGBB). */
    return b | ( g << 8 ) | ( r << 16 ) | 0xff000000u;
}

static void xbox_build_565( void )
{
    int i;
    if ( xbox_rgb565_ready ) return;
    for ( i = 0; i < 65536; i++ )
        xbox_rgb565[i] = xbox_from_565( ( uint16_t )i );
    xbox_rgb565_ready = 1;
}

static void xbox_video_info( void )
{
    VIDEO_MODE mode = XVideoGetMode();
    xbox_fb = XVideoGetFB();
    xbox_width = mode.width;
    xbox_height = mode.height;
    xbox_bpp = mode.bpp;
    xbox_pitch = xbox_width * ( xbox_bpp / 8 );
}

static int xbox_build_scale_maps( int source_width, int source_height )
{
    int x, y;

    if ( xbox_source_width == source_width &&
         xbox_source_height == source_height &&
         xbox_scale_x && xbox_scale_y )
        return 1;

    free( xbox_scale_x );
    free( xbox_scale_y );
    xbox_scale_x = malloc( ( size_t )xbox_width * sizeof( *xbox_scale_x ) );
    xbox_scale_y = malloc( ( size_t )xbox_height * sizeof( *xbox_scale_y ) );
    if ( !xbox_scale_x || !xbox_scale_y ) return 0;

    for ( x = 0; x < xbox_width; x++ )
        xbox_scale_x[x] = ( int )( ( int64_t )x * source_width / xbox_width );
    for ( y = 0; y < xbox_height; y++ )
        xbox_scale_y[y] = ( int )( ( int64_t )y * source_height / xbox_height );

    xbox_source_width = source_width;
    xbox_source_height = source_height;
    return 1;
}

void gr_video_xbox_module_initialize( void )
{
    /* Surfaces work without a real window; still init the dummy video driver. */
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_InitSubSystem( SDL_INIT_VIDEO );
    if ( !XVideoGetFB() )
        XVideoSetMode( 640, 480, 32, REFRESH_DEFAULT );
    xbox_video_info();
}

void gr_video_xbox_destroy( void )
{
    free( xbox_scale_x );
    free( xbox_scale_y );
    xbox_scale_x = NULL;
    xbox_scale_y = NULL;
    xbox_source_width = 0;
    xbox_source_height = 0;
    xbox_fb = NULL;
}

void gr_video_xbox_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

static int xbox_present_surface( SDL_Surface * src )
{
    int x, y, bpp;
    uint8_t * dst;
    uint8_t * src8;
    uint16_t * src16;
    uint32_t * src32;
    uint32_t * dst32;

    if ( !src || !src->pixels ) return 0;
    xbox_video_info();
    if ( !xbox_fb || xbox_width <= 0 || xbox_height <= 0 ) return 0;
    if ( !xbox_build_scale_maps( src->w, src->h ) ) return 0;

    bpp = bennu_surface_bpp( src );
    xbox_build_565();
    dst = xbox_fb;

    for ( y = 0; y < xbox_height; y++ )
    {
        int sy = xbox_scale_y[y];
        dst32 = ( uint32_t * )( dst + y * xbox_pitch );
        if ( bpp == 8 )
        {
            SDL_Palette * pal = bennu_surface_palette( src );
            src8 = ( uint8_t * )src->pixels + sy * src->pitch;
            for ( x = 0; x < xbox_width; x++ )
            {
                uint8_t idx = src8[xbox_scale_x[x]];
                SDL_Color c = { 0, 0, 0, 255 };
                if ( pal && idx < pal->ncolors )
                    c = pal->colors[idx];
                dst32[x] = ( uint32_t )c.b | ( ( uint32_t )c.g << 8 ) |
                           ( ( uint32_t )c.r << 16 ) | 0xff000000u;
            }
        }
        else if ( bpp == 16 )
        {
            src16 = ( uint16_t * )( ( uint8_t * )src->pixels + sy * src->pitch );
            for ( x = 0; x < xbox_width; x++ )
                dst32[x] = xbox_rgb565[src16[xbox_scale_x[x]]];
        }
        else
        {
            src32 = ( uint32_t * )( ( uint8_t * )src->pixels + sy * src->pitch );
            for ( x = 0; x < xbox_width; x++ )
            {
                uint32_t p = src32[xbox_scale_x[x]];
                /* SDL RGBA8888 / ABGR pack → A8R8G8B8 dword. */
                dst32[x] = ( ( p & 0x000000ffu ) << 16 ) |
                           ( p & 0x0000ff00u ) |
                           ( ( p & 0x00ff0000u ) >> 16 ) |
                           0xff000000u;
            }
        }
    }

    XVideoWaitForVBlank();
    XVideoFlushFB();
    return 1;
}

int gr_video_xbox_present( SDL_Surface * src )
{
    return xbox_present_surface( src );
}

int gr_video_xbox_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void )rects;
    ( void )count;
    return xbox_present_surface( src );
}
