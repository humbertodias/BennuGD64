/* PlayStation 5 VideoOut presenter for Bennu's software framebuffer. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "libvideo.h"
#include "g_video_ps5.h"
#include "ps5_sce.h"

#ifdef BENNUGD_PS5_NATIVE_TITLE
#include "ps5_log.h"
#define PS5_TILE_WIDTH 512
#define PS5_TILE_HEIGHT 128
#define PS5_TILE_PIXELS (PS5_TILE_WIDTH * PS5_TILE_HEIGHT)
#define PS5_FLIP_TIMEOUT_US 100000
#endif

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
#ifdef BENNUGD_PS5_NATIVE_TITLE
    int flip_pending;
    uint32_t tile_column[ PS5_FB_MAX_W / 4 ];
    uint32_t tile_row[ PS5_TILE_HEIGHT ];
#endif
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

#ifdef BENNUGD_PS5_NATIVE_TITLE
/*
 * VideoOut address layout adapted from SDL_ps5tilemap.c (ps5-payload-dev/SDL).
 * Copyright (C) 2026 John Törnblom <john.tornblom@gmail.com>.
 * The zlib license in LICENSE applies to this adaptation.
 */
static uint32_t ps5_tile_offset( unsigned int x, unsigned int y )
{
    static const unsigned int x_bits[] = { 0, 1, 5, 6, 7, 9, 8, 14, 15 };
    static const unsigned int y_bits[] = { 2, 3, 4, 6, 7, 8, 9 };
    uint32_t offset = ((x & 0x10u) << 7) | ((x & 0x40u) << 7) |
                      ((y & 0x08u) << 7) | ((y & 0x40u) << 6);

    for ( unsigned int bit = 0; bit < sizeof( x_bits ) / sizeof( x_bits[0] ); bit++ )
        offset ^= ((x >> bit) & 1u) << x_bits[bit];
    for ( unsigned int bit = 0; bit < sizeof( y_bits ) / sizeof( y_bits[0] ); bit++ )
        offset ^= ((y >> bit) & 1u) << y_bits[bit];
    return offset;
}

static size_t ps5_prepare_tiled_buffer( int width, int height )
{
    for ( int x = 0; x < width; x += 4 )
        ps5_video.tile_column[x / 4] = (x / PS5_TILE_WIDTH) * PS5_TILE_PIXELS +
                                      ps5_tile_offset( x % PS5_TILE_WIDTH, 0 );
    for ( int y = 0; y < PS5_TILE_HEIGHT; y++ )
        ps5_video.tile_row[y] = ps5_tile_offset( 0, y );

    size_t last_band = (size_t)((height - 1) / PS5_TILE_HEIGHT) * PS5_TILE_HEIGHT * width;
    size_t final_band_size = (size_t)((width - 1) / PS5_TILE_WIDTH + 1) * PS5_TILE_PIXELS;
    return (last_band + final_band_size) * sizeof( uint32_t );
}

static void ps5_video_error( const char * operation, int result )
{
    static const char * previous_operation;
    static int previous_result;
    char message[128];

    if ( previous_operation == operation && previous_result == result )
        return;
    previous_operation = operation;
    previous_result = result;
    snprintf( message, sizeof( message ), "bgdi: VideoOut %s failed (0x%08x)",
              operation, (unsigned int) result );
    ps5_log_write( message );
}

#else
static void ps5_video_error( const char * operation, int result )
{
    (void) operation;
    (void) result;
}
#endif

#define PS5_VIDEO_STEP(operation, call) do { \
    rc = (call); \
    if ( rc != 0 ) { ps5_video_error( operation, rc ); goto fail; } \
} while (0)

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
    {
        ps5_video_error( "sceVideoOutOpen", ps5_video.handle );
        goto fail;
    }

    memset( &status, 0, sizeof( status ) );
    PS5_VIDEO_STEP( "sceVideoOutGetOutputStatus", sceVideoOutGetOutputStatus( ps5_video.handle, &status ) );
    if ( status.resolution == 2 )
    {
        ps5_video.width = 3840;
        ps5_video.height = 2160;
    }

#ifdef BENNUGD_PS5_NATIVE_TITLE
    ps5_video.frame_bytes = ps5_prepare_tiled_buffer( ps5_video.width, ps5_video.height );
#else
    ps5_video.frame_bytes = ( size_t ) ps5_video.width * ( size_t ) ps5_video.height * sizeof( uint32_t );
#endif
    stride = ps5_align_up( ps5_video.frame_bytes, PS5_FB_ALIGN );
    ps5_video.mapped_size = stride * PS5_FB_COUNT;

    PS5_VIDEO_STEP( "sceKernelAllocateMainDirectMemory",
                    sceKernelAllocateMainDirectMemory( ps5_video.mapped_size, PS5_FB_ALIGN,
                                                       PS5_MEM_TYPE, &ps5_video.phys ) );
    ps5_video.allocated = 1;

    PS5_VIDEO_STEP( "sceKernelMapDirectMemory",
                    sceKernelMapDirectMemory( &ps5_video.mapped, ps5_video.mapped_size,
                                              PS5_MEM_PROT, 0, ps5_video.phys, PS5_FB_ALIGN ) );
    memset( ps5_video.mapped, 0, ps5_video.mapped_size );

    ps5_video.buffers[ 0 ].data = ps5_video.mapped;
    ps5_video.buffers[ 1 ].data = ( uint8_t * ) ps5_video.mapped + stride;

    memset( &attr, 0, sizeof( attr ) );
    sceVideoOutSetBufferAttribute2( &attr, SCE_VIDEO_OUT_PIXEL_RGBA8, 0,
                                   ( uint32_t ) ps5_video.width,
                                   ( uint32_t ) ps5_video.height,
                                   0, 0, 0 );
#ifdef BENNUGD_PS5_NATIVE_TITLE
    PS5_VIDEO_STEP( "sceKernelCreateEqueue", sceKernelCreateEqueue( &ps5_video.flip_queue, "bennugd64-video" ) );
    PS5_VIDEO_STEP( "sceVideoOutAddFlipEvent", sceVideoOutAddFlipEvent( ps5_video.flip_queue, ps5_video.handle, NULL ) );
#endif
    PS5_VIDEO_STEP( "sceVideoOutRegisterBuffers2",
                    sceVideoOutRegisterBuffers2( ps5_video.handle, 0, 0, ps5_video.buffers,
                                                PS5_FB_COUNT, &attr, 0, NULL ) );
    ps5_video.registered = 1;
#ifdef BENNUGD_PS5_NATIVE_TITLE
    PS5_VIDEO_STEP( "sceVideoOutSetFlipRate", sceVideoOutSetFlipRate( ps5_video.handle, 0 ) );
    ps5_log_write( "bgdi: VideoOut buffers registered" );
#else
    PS5_VIDEO_STEP( "sceKernelCreateEqueue", sceKernelCreateEqueue( &ps5_video.flip_queue, "bennugd64-video" ) );
    PS5_VIDEO_STEP( "sceVideoOutAddFlipEvent", sceVideoOutAddFlipEvent( ps5_video.flip_queue, ps5_video.handle, NULL ) );
    sceVideoOutSetFlipRate( ps5_video.handle, 0 );
#endif
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

#ifdef BENNUGD_PS5_NATIVE_TITLE
        uint32_t * band = dst + (y / PS5_TILE_HEIGHT) * PS5_TILE_HEIGHT * ps5_video.width;
        uint32_t row = ps5_video.tile_row[y % PS5_TILE_HEIGHT];
        for ( int x = 0; x < ps5_video.width; x += 4 )
            memcpy( band + (ps5_video.tile_column[x / 4] ^ row),
                    ps5_scaled_row + x, 4 * sizeof( uint32_t ) );
#else
        memcpy( dst + y * ps5_video.width, ps5_scaled_row,
                ( size_t ) ps5_video.width * sizeof( *ps5_scaled_row ) );
#endif
    }
}

#ifdef BENNUGD_PS5_NATIVE_TITLE
static int ps5_video_wait_flip( void )
{
    uint64_t event[32];
    unsigned int timeout = PS5_FLIP_TIMEOUT_US;
    int out = 0;
    int rc = sceKernelWaitEqueue( ps5_video.flip_queue, event, 1, &out, &timeout );
    if ( rc != 0 || !out )
    {
        ps5_video_error( "waiting for flip", rc );
        return 0;
    }
    ps5_video.flip_pending = 0;
    ps5_video.current = (ps5_video.current + 1) % PS5_FB_COUNT;
    if ( ps5_video.flip_arg == 1 )
        ps5_log_write( "bgdi: VideoOut first flip completed" );
    return 1;
}
#endif

int gr_video_ps5_present( SDL_Surface * src )
{
#ifndef BENNUGD_PS5_NATIVE_TITLE
    unsigned char event[ 256 ];
    int out = 0;
#endif
    int rc;

    if ( !src || !src->pixels || src->w < 1 || src->h < 1 )
        return 0;
    if ( !ps5_video_initialize() )
        return 0;

#ifdef BENNUGD_PS5_NATIVE_TITLE
    if ( ps5_video.flip_pending && !ps5_video_wait_flip() )
        return 0;
#endif
    ps5_blit_scaled( src, ( uint32_t * ) ps5_video.buffers[ ps5_video.current ].data );
#ifdef BENNUGD_PS5_NATIVE_TITLE
    /* The scan-out buffers are write-combining direct memory. */
    __builtin_ia32_sfence();
#endif
    rc = sceVideoOutSubmitFlip( ps5_video.handle, ps5_video.current,
                                SCE_VIDEO_OUT_FLIP_VSYNC,
                                ( int64_t ) ++ps5_video.flip_arg );
    if ( rc != 0 )
    {
#ifdef BENNUGD_PS5_NATIVE_TITLE
        ps5_video_error( "sceVideoOutSubmitFlip", rc );
#endif
        return 0;
    }
#ifdef BENNUGD_PS5_NATIVE_TITLE
    ps5_video.flip_pending = 1;
    if ( ps5_video.flip_arg == 1 )
        ps5_log_write( "bgdi: VideoOut first flip submitted" );
    return ps5_video_wait_flip();
#else
    rc = sceKernelWaitEqueue( ps5_video.flip_queue, event, 1, &out, NULL );
    if ( rc != 0 )
        return 0;
    ps5_video.current = ( ps5_video.current + 1 ) % PS5_FB_COUNT;
    return 1;
#endif
}

int gr_video_ps5_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps5_present( src );
}
