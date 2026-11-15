/* PlayStation 5 VideoOut presenter for Bennu's software framebuffer. */

#include <stdint.h>
#include <string.h>

#include "libvideo.h"
#include "g_video_ps5.h"
#include "ps5_sce.h"

#define PS5_FB_MAX_W     3840
#define PS5_FB_MAX_H     2160
#define PS5_FB_COUNT     2
#define PS5_FB_ALIGN     0x20000
#define PS5_MEM_TYPE     3
#define PS5_MEM_PROT     0x33

typedef struct Ps5VideoState
{
    int handle;
    int current;
    int width;
    int height;
    int source_width;
    int source_height;
    int allocated;
    int registered;
    uint64_t flip_arg;
    intptr_t phys;
    void * mapped;
    SceVideoOutBuffer buffers[ PS5_FB_COUNT ];
    size_t frame_bytes;
    size_t mapped_size;
    void * flip_queue;
    int scale_x[ PS5_FB_MAX_W ];
    int scale_y[ PS5_FB_MAX_H ];
} Ps5VideoState;

static Ps5VideoState ps5_video = {
    .handle = -1
};

static uint32_t ps5_scaled_row[ PS5_FB_MAX_W ] __attribute__((aligned(64)));
static uint32_t ps5_rgb565_table[ 65536 ] __attribute__((aligned(64)));
static int ps5_rgb565_table_ready;

static size_t ps5_align_up( size_t value, size_t alignment )
{
    return ( value + alignment - 1 ) & ~( alignment - 1 );
}

static void ps5_video_reset( void )
{
    memset( &ps5_video, 0, sizeof( ps5_video ) );
    ps5_video.handle = -1;
}

void gr_video_ps5_destroy( void )
{
    if ( ps5_video.handle >= 0 && ps5_video.registered )
        sceVideoOutUnregisterBuffers( ps5_video.handle, 0 );
    if ( ps5_video.handle >= 0 )
        sceVideoOutClose( ps5_video.handle );
    if ( ps5_video.flip_queue )
        sceKernelDeleteEqueue( ps5_video.flip_queue );
    if ( ps5_video.mapped )
        sceKernelMunmap( ps5_video.mapped, ps5_video.mapped_size );
    if ( ps5_video.allocated )
        sceKernelReleaseDirectMemory( ps5_video.phys, ps5_video.mapped_size );
    ps5_video_reset();
}

static int ps5_video_initialize( void )
{
    SceVideoOutBufferAttribute2 attr;
    SceVideoOutStatus status;
    size_t stride;
    int rc;

    if ( ps5_video.handle >= 0 )
        return 1;

    ps5_video_reset();
    ps5_video.width = 1920;
    ps5_video.height = 1080;

    ps5_video.handle = sceVideoOutOpen( 0xff, SCE_VIDEO_OUT_BUS_MAIN, 0, NULL );
    if ( ps5_video.handle < 0 )
        goto fail;

    memset( &status, 0, sizeof( status ) );
    rc = sceVideoOutGetOutputStatus( ps5_video.handle, &status );
    if ( rc != 0 )
        goto fail;
    if ( status.resolution == 2 )
    {
        ps5_video.width = 3840;
        ps5_video.height = 2160;
    }

    ps5_video.frame_bytes = ( size_t ) ps5_video.width * ( size_t ) ps5_video.height * sizeof( uint32_t );
    stride = ps5_align_up( ps5_video.frame_bytes, PS5_FB_ALIGN );
    ps5_video.mapped_size = stride * PS5_FB_COUNT;

    rc = sceKernelAllocateMainDirectMemory( ps5_video.mapped_size, PS5_FB_ALIGN,
                                            PS5_MEM_TYPE, &ps5_video.phys );
    if ( rc != 0 )
        goto fail;
    ps5_video.allocated = 1;

    rc = sceKernelMapDirectMemory( &ps5_video.mapped, ps5_video.mapped_size,
                                   PS5_MEM_PROT, 0, ps5_video.phys, PS5_FB_ALIGN );
    if ( rc != 0 )
        goto fail;
    memset( ps5_video.mapped, 0, ps5_video.mapped_size );

    ps5_video.buffers[ 0 ].data = ps5_video.mapped;
    ps5_video.buffers[ 1 ].data = ( uint8_t * ) ps5_video.mapped + stride;

    memset( &attr, 0, sizeof( attr ) );
    sceVideoOutSetBufferAttribute2( &attr, SCE_VIDEO_OUT_PIXEL_RGBA8, 0,
                                   ( uint32_t ) ps5_video.width,
                                   ( uint32_t ) ps5_video.height,
                                   0, 0, 0 );
    rc = sceVideoOutRegisterBuffers2( ps5_video.handle, 0, 0, ps5_video.buffers,
                                     PS5_FB_COUNT, &attr, 0, NULL );
    if ( rc != 0 )
        goto fail;
    ps5_video.registered = 1;

    rc = sceKernelCreateEqueue( &ps5_video.flip_queue, "bennugd64-video" );
    if ( rc != 0 )
        goto fail;
    rc = sceVideoOutAddFlipEvent( ps5_video.flip_queue, ps5_video.handle, NULL );
    if ( rc != 0 )
        goto fail;
    sceVideoOutSetFlipRate( ps5_video.handle, 0 );
    return 1;

fail:
    gr_video_ps5_destroy();
    return 0;
}

void gr_video_ps5_module_initialize( void )
{
    ps5_video_initialize();
}

void gr_video_ps5_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    if ( ps5_video.width < 1 )
    {
        *width = 1920;
        *height = 1080;
    }
    else
    {
        *width = ps5_video.width;
        *height = ps5_video.height;
    }
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_ps5_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

static uint32_t ps5_rgb565( uint16_t pixel )
{
    uint32_t r = ( pixel >> 11 ) & 0x1f;
    uint32_t g = ( pixel >> 5 ) & 0x3f;
    uint32_t b = pixel & 0x1f;

    r = ( r << 3 ) | ( r >> 2 );
    g = ( g << 2 ) | ( g >> 4 );
    b = ( b << 3 ) | ( b >> 2 );
    return r | ( g << 8 ) | ( b << 16 ) | 0xff000000u;
}

static void ps5_build_rgb565_table( void )
{
    int pixel;

    if ( ps5_rgb565_table_ready )
        return;

    for ( pixel = 0; pixel < 65536; pixel++ )
        ps5_rgb565_table[ pixel ] = ps5_rgb565( ( uint16_t ) pixel );
    ps5_rgb565_table_ready = 1;
}

static void ps5_build_scale_maps( int width, int height )
{
    int x, y;

    if ( ps5_video.source_width != width )
    {
        for ( x = 0; x < ps5_video.width; x++ )
            ps5_video.scale_x[ x ] =
                ( int )( ( int64_t ) x * width / ps5_video.width );
        ps5_video.source_width = width;
    }

    if ( ps5_video.source_height != height )
    {
        for ( y = 0; y < ps5_video.height; y++ )
            ps5_video.scale_y[ y ] =
                ( int )( ( int64_t ) y * height / ps5_video.height );
        ps5_video.source_height = height;
    }
}

static void ps5_scale_row_565( const uint16_t * src )
{
    int x;

    for ( x = 0; x < ps5_video.width; x++ )
        ps5_scaled_row[ x ] = ps5_rgb565_table[ src[ ps5_video.scale_x[ x ] ] ];
}

static void ps5_scale_row_32( const uint32_t * src )
{
    int x;

    for ( x = 0; x < ps5_video.width; x++ )
    {
        uint32_t pixel = src[ ps5_video.scale_x[ x ] ];
        ps5_scaled_row[ x ] = ( pixel & 0xff00ff00u ) |
                              ( ( pixel & 0x00ff0000u ) >> 16 ) |
                              ( ( pixel & 0x000000ffu ) << 16 ) |
                              0xff000000u;
    }
}

static void ps5_blit_scaled( SDL_Surface * src, uint32_t * dst )
{
    int y;
    int last_sy = -1;
    int bpp = bennu_surface_bytes_pp( src );

    ps5_build_scale_maps( src->w, src->h );
    if ( bpp == 2 )
        ps5_build_rgb565_table();

    for ( y = 0; y < ps5_video.height; y++ )
    {
        int sy = ps5_video.scale_y[ y ];

        if ( sy != last_sy )
        {
            const uint8_t * row =
                ( const uint8_t * ) src->pixels + sy * src->pitch;

            if ( bpp == 2 )
                ps5_scale_row_565( ( const uint16_t * ) row );
            else
                ps5_scale_row_32( ( const uint32_t * ) row );
            last_sy = sy;
        }

        memcpy( dst + y * ps5_video.width, ps5_scaled_row,
                ( size_t ) ps5_video.width * sizeof( *ps5_scaled_row ) );
    }
}

int gr_video_ps5_present( SDL_Surface * src )
{
    unsigned char event[ 256 ];
    int out = 0;
    int rc;

    if ( !src || !src->pixels || src->w < 1 || src->h < 1 )
        return 0;
    if ( !ps5_video_initialize() )
        return 0;

    ps5_blit_scaled( src, ( uint32_t * ) ps5_video.buffers[ ps5_video.current ].data );
    rc = sceVideoOutSubmitFlip( ps5_video.handle, ps5_video.current,
                                SCE_VIDEO_OUT_FLIP_VSYNC,
                                ( int64_t ) ++ps5_video.flip_arg );
    if ( rc != 0 )
        return 0;
    rc = sceKernelWaitEqueue( ps5_video.flip_queue, event, 1, &out, NULL );
    if ( rc != 0 )
        return 0;
    ps5_video.current = ( ps5_video.current + 1 ) % PS5_FB_COUNT;
    return 1;
}

int gr_video_ps5_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps5_present( src );
}
