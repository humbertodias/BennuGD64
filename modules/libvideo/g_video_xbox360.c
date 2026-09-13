/* libXenon tiled-framebuffer presenter for Bennu's software surface. */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cache.h>
#include <console/console.h>

#include "libvideo.h"
#include "g_video_xbox360.h"

typedef struct Xbox360AtiInfo
{
    uint32_t unknown1[4];
    uint32_t base;
    uint32_t unknown2[8];
    uint32_t width;
    uint32_t height;
} __attribute__((packed)) Xbox360AtiInfo;

static uint32_t * xbox360_fb;
static int xbox360_width;
static int xbox360_height;
static int xbox360_pitch;
static int xbox360_console_closed;
static int * xbox360_scale_x;
static int * xbox360_scale_y;
static int xbox360_source_width;
static int xbox360_source_height;
static uint32_t xbox360_rgb565[65536] __attribute__((aligned(128)));
static int xbox360_rgb565_ready;

static void xbox360_video_info( void )
{
    volatile Xbox360AtiInfo * info = ( volatile Xbox360AtiInfo * )0xec806100u;
    xbox360_fb = ( uint32_t * )( uintptr_t )( info->base | 0x80000000u );
    xbox360_width = ( int )info->width;
    xbox360_height = ( int )info->height;
    xbox360_pitch = ( xbox360_width + 31 ) & ~31;
}

static unsigned int xbox360_tiled_index( int x, int y )
{
    return ( unsigned int )(
        ( ( y >> 5 ) * 32 * xbox360_pitch ) +
        ( ( x >> 5 ) << 10 ) +
        ( x & 3 ) +
        ( ( y & 1 ) << 2 ) +
        ( ( ( x & 31 ) >> 2 ) << 3 ) +
        ( ( ( y & 31 ) >> 1 ) << 6 )
    ) ^ ( unsigned int )( ( y & 8 ) << 2 );
}

static uint32_t xbox360_from_565( uint16_t pixel )
{
    uint32_t r = ( pixel >> 11 ) & 31u;
    uint32_t g = ( pixel >> 5 ) & 63u;
    uint32_t b = pixel & 31u;
    r = ( r << 3 ) | ( r >> 2 );
    g = ( g << 2 ) | ( g >> 4 );
    b = ( b << 3 ) | ( b >> 2 );
    return ( b << 24 ) | ( g << 16 ) | ( r << 8 );
}

static void xbox360_build_565( void )
{
    int i;
    if ( xbox360_rgb565_ready ) return;
    for ( i = 0; i < 65536; i++ )
        xbox360_rgb565[i] = xbox360_from_565( ( uint16_t )i );
    xbox360_rgb565_ready = 1;
}

static int xbox360_build_scale_maps( int source_width, int source_height )
{
    int x, y;

    if ( xbox360_source_width == source_width &&
         xbox360_source_height == source_height &&
         xbox360_scale_x && xbox360_scale_y )
        return 1;

    free( xbox360_scale_x );
    free( xbox360_scale_y );
    xbox360_scale_x = malloc( ( size_t )xbox360_width * sizeof( *xbox360_scale_x ) );
    xbox360_scale_y = malloc( ( size_t )xbox360_height * sizeof( *xbox360_scale_y ) );
    if ( !xbox360_scale_x || !xbox360_scale_y ) return 0;

    for ( x = 0; x < xbox360_width; x++ )
        xbox360_scale_x[x] = ( int )( ( int64_t )x * source_width / xbox360_width );
    for ( y = 0; y < xbox360_height; y++ )
        xbox360_scale_y[y] = ( int )( ( int64_t )y * source_height / xbox360_height );

    xbox360_source_width = source_width;
    xbox360_source_height = source_height;
    return 1;
}

void gr_video_xbox360_module_initialize( void )
{
    xbox360_video_info();
}

void gr_video_xbox360_destroy( void )
{
    free( xbox360_scale_x );
    free( xbox360_scale_y );
    xbox360_scale_x = NULL;
    xbox360_scale_y = NULL;
    xbox360_source_width = 0;
    xbox360_source_height = 0;
    xbox360_fb = NULL;
}

void gr_video_xbox360_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

int gr_video_xbox360_present( SDL_Surface * src )
{
    int x, y;
    int bpp;
    SDL_Palette * palette;

    if ( !src || !src->pixels || src->w < 1 || src->h < 1 ) return 0;
    if ( !xbox360_fb ) xbox360_video_info();
    if ( !xbox360_fb || xbox360_width < 1 || xbox360_height < 1 ) return 0;
    if ( !xbox360_build_scale_maps( src->w, src->h ) ) return 0;
    if ( !xbox360_console_closed )
    {
        console_close();
        xbox360_console_closed = 1;
    }

    bpp = bennu_surface_bytes_pp( src );
    palette = bennu_surface_palette( src );
    if ( bpp == 2 ) xbox360_build_565();

    for ( y = 0; y < xbox360_height; y++ )
    {
        int sy = xbox360_scale_y[y];
        const uint8_t * row = ( const uint8_t * )src->pixels + sy * src->pitch;
        for ( x = 0; x < xbox360_width; x++ )
        {
            int sx = xbox360_scale_x[x];
            uint32_t pixel;
            if ( bpp == 2 )
            {
                pixel = xbox360_rgb565[( ( const uint16_t * )row )[sx]];
            }
            else if ( bpp == 1 && palette )
            {
                SDL_Color color = palette->colors[row[sx]];
                pixel = ( ( uint32_t )color.b << 24 ) |
                        ( ( uint32_t )color.g << 16 ) |
                        ( ( uint32_t )color.r << 8 );
            }
            else
            {
                uint32_t source = ( ( const uint32_t * )row )[sx];
                pixel = ( ( source & 0xff000000u ) >> 16 ) |
                        ( source & 0x00ff0000u ) |
                        ( ( source & 0x0000ff00u ) << 16 );
            }
            xbox360_fb[xbox360_tiled_index( x, y )] = pixel;
        }
    }

    memdcbst( xbox360_fb, xbox360_pitch * ( ( xbox360_height + 31 ) & ~31 ) * 4 );
    return 1;
}

int gr_video_xbox360_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void )rects;
    ( void )count;
    return gr_video_xbox360_present( src );
}
