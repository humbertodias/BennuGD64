/* PlayStation 4 VideoOut presenter for Bennu's software framebuffer. */

#include <stdint.h>
#include <string.h>

#include <orbis/libkernel.h>
#include <orbis/VideoOut.h>

#include "libvideo.h"
#include "g_video_ps4.h"

#define PS4_FB_WIDTH       1920
#define PS4_FB_HEIGHT      1080
#define PS4_FB_COUNT       2
#define PS4_FB_ALIGNMENT   ( 64 * 1024 )

typedef struct Ps4VideoState
{
    int handle;
    int current;
    int source_width;
    int source_height;
    int allocated;
    int registered;
    uint64_t flip_arg;
    off_t phys;
    void * mapped;
    void * buffers[ PS4_FB_COUNT ];
    size_t frame_size;
    size_t mapped_size;
    OrbisKernelEqueue flip_queue;
    int scale_x[ PS4_FB_WIDTH ];
    int scale_y[ PS4_FB_HEIGHT ];
} Ps4VideoState;

static Ps4VideoState ps4_video = {
    .handle = -1
};

static uint32_t ps4_scaled_row[ PS4_FB_WIDTH ] __attribute__((aligned(64)));
static uint32_t ps4_rgb565_table[ 65536 ] __attribute__((aligned(64)));
static int ps4_rgb565_table_ready;

static size_t ps4_align_up( size_t value, size_t alignment )
{
    return ( value + alignment - 1 ) & ~( alignment - 1 );
}

static void ps4_video_reset( void )
{
    memset( &ps4_video, 0, sizeof( ps4_video ) );
    ps4_video.handle = -1;
}

void gr_video_ps4_destroy( void )
{
    int i;

    if ( ps4_video.handle >= 0 && ps4_video.registered )
    {
        for ( i = 0; i < PS4_FB_COUNT; i++ )
            sceVideoOutUnregisterBuffers( ps4_video.handle, i );
    }
    if ( ps4_video.handle >= 0 )
        sceVideoOutClose( ps4_video.handle );
    if ( ps4_video.flip_queue )
        sceKernelDeleteEqueue( ps4_video.flip_queue );
    if ( ps4_video.mapped )
        sceKernelMunmap( ps4_video.mapped, ps4_video.mapped_size );
    if ( ps4_video.allocated )
        sceKernelReleaseDirectMemory( ps4_video.phys, ps4_video.mapped_size );
    ps4_video_reset();
}

static int ps4_video_initialize( void )
{
    OrbisVideoOutBufferAttribute attr;
    OrbisVideoOutResolutionStatus status;
    int rc, i;

    if ( ps4_video.handle >= 0 )
        return 1;

    ps4_video_reset();
    ps4_video.frame_size = ( size_t ) PS4_FB_WIDTH * PS4_FB_HEIGHT * sizeof( uint32_t );
    ps4_video.mapped_size = ps4_align_up( ps4_video.frame_size, PS4_FB_ALIGNMENT ) * PS4_FB_COUNT;

    ps4_video.handle = sceVideoOutOpen( 0, ORBIS_VIDEO_OUT_BUS_MAIN, 0, NULL );
    if ( ps4_video.handle < 0 )
        goto fail;

    memset( &status, 0, sizeof( status ) );
    rc = sceVideoOutGetResolutionStatus( ps4_video.handle, &status );
    if ( rc != 0 )
        goto fail;

    rc = sceKernelAllocateDirectMemory( 0, ( off_t ) sceKernelGetDirectMemorySize(),
                                        ps4_video.mapped_size, PS4_FB_ALIGNMENT,
                                        ORBIS_KERNEL_WC_GARLIC, &ps4_video.phys );
    if ( rc != 0 )
        goto fail;
    ps4_video.allocated = 1;

    rc = sceKernelMapDirectMemory( &ps4_video.mapped, ps4_video.mapped_size,
                                   ORBIS_KERNEL_PROT_CPU_READ |
                                   ORBIS_KERNEL_PROT_CPU_RW |
                                   ORBIS_KERNEL_PROT_GPU_READ,
                                   0, ps4_video.phys, PS4_FB_ALIGNMENT );
    if ( rc != 0 )
        goto fail;

    for ( i = 0; i < PS4_FB_COUNT; i++ )
        ps4_video.buffers[ i ] = ( uint8_t * ) ps4_video.mapped +
                                  ps4_align_up( ps4_video.frame_size, PS4_FB_ALIGNMENT ) * i;
    memset( ps4_video.mapped, 0, ps4_video.mapped_size );

    memset( &attr, 0, sizeof( attr ) );
    sceVideoOutSetBufferAttribute( &attr,
                                   ORBIS_VIDEO_OUT_PIXEL_FORMAT_A8B8G8R8_SRGB,
                                   ORBIS_VIDEO_OUT_TILING_MODE_LINEAR,
                                   ORBIS_VIDEO_OUT_ASPECT_RATIO_16_9,
                                   PS4_FB_WIDTH, PS4_FB_HEIGHT, PS4_FB_WIDTH );
    rc = sceVideoOutRegisterBuffers( ps4_video.handle, 0, ps4_video.buffers,
                                     PS4_FB_COUNT, &attr );
    if ( rc < 0 )
        goto fail;
    ps4_video.registered = 1;

    rc = sceKernelCreateEqueue( &ps4_video.flip_queue, "bennugd64-video" );
    if ( rc != 0 )
        goto fail;
    rc = sceVideoOutAddFlipEvent( ps4_video.flip_queue, ps4_video.handle, NULL );
    if ( rc != 0 )
        goto fail;
    sceVideoOutSetFlipRate( ps4_video.handle, ORBIS_VIDEO_OUT_FLIP_60HZ );
    return 1;

fail:
    gr_video_ps4_destroy();
    return 0;
}

void gr_video_ps4_module_initialize( void )
{
    /* Software surfaces do not require SDL's Linux/dummy video subsystem. */
    ps4_video_initialize();
}

void gr_video_ps4_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    *width = PS4_FB_WIDTH;
    *height = PS4_FB_HEIGHT;
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_ps4_apply_mode( void )
{
    full_screen = 1;
    waitvsync = 0;
    enable_scale = 0;
    scale_mode = SCALE_NONE;
}

static uint32_t ps4_rgb565( uint16_t pixel )
{
    uint32_t r = ( pixel >> 11 ) & 0x1f;
    uint32_t g = ( pixel >> 5 ) & 0x3f;
    uint32_t b = pixel & 0x1f;

    r = ( r << 3 ) | ( r >> 2 );
    g = ( g << 2 ) | ( g >> 4 );
    b = ( b << 3 ) | ( b >> 2 );
    return r | ( g << 8 ) | ( b << 16 ) | 0xff000000u;
}

static void ps4_build_rgb565_table( void )
{
    int pixel;

    if ( ps4_rgb565_table_ready )
        return;

    for ( pixel = 0; pixel < 65536; pixel++ )
        ps4_rgb565_table[ pixel ] = ps4_rgb565( ( uint16_t ) pixel );
    ps4_rgb565_table_ready = 1;
}

static void ps4_build_scale_maps( int width, int height )
{
    int x, y;

    if ( ps4_video.source_width != width )
    {
        for ( x = 0; x < PS4_FB_WIDTH; x++ )
            ps4_video.scale_x[ x ] =
                ( int )( ( int64_t ) x * width / PS4_FB_WIDTH );
        ps4_video.source_width = width;
    }

    if ( ps4_video.source_height != height )
    {
        for ( y = 0; y < PS4_FB_HEIGHT; y++ )
            ps4_video.scale_y[ y ] =
                ( int )( ( int64_t ) y * height / PS4_FB_HEIGHT );
        ps4_video.source_height = height;
    }
}

static void ps4_scale_row_565( const uint16_t * src )
{
    int x;

    for ( x = 0; x < PS4_FB_WIDTH; x++ )
        ps4_scaled_row[ x ] = ps4_rgb565_table[ src[ ps4_video.scale_x[ x ] ] ];
}

static void ps4_scale_row_32( const uint32_t * src )
{
    int x;

    for ( x = 0; x < PS4_FB_WIDTH; x++ )
    {
        uint32_t pixel = src[ ps4_video.scale_x[ x ] ];
        ps4_scaled_row[ x ] = ( pixel & 0xff00ff00u ) |
                              ( ( pixel & 0x00ff0000u ) >> 16 ) |
                              ( ( pixel & 0x000000ffu ) << 16 ) |
                              0xff000000u;
    }
}

static void ps4_blit_scaled( SDL_Surface * src, uint32_t * dst )
{
    int y;
    int last_sy = -1;
    int bpp = bennu_surface_bytes_pp( src );

    ps4_build_scale_maps( src->w, src->h );
    if ( bpp == 2 )
        ps4_build_rgb565_table();

    for ( y = 0; y < PS4_FB_HEIGHT; y++ )
    {
        int sy = ps4_video.scale_y[ y ];

        if ( sy != last_sy )
        {
            const uint8_t * row =
                ( const uint8_t * ) src->pixels + sy * src->pitch;

            if ( bpp == 2 )
                ps4_scale_row_565( ( const uint16_t * ) row );
            else
                ps4_scale_row_32( ( const uint32_t * ) row );
            last_sy = sy;
        }

        memcpy( dst + y * PS4_FB_WIDTH, ps4_scaled_row,
                PS4_FB_WIDTH * sizeof( *ps4_scaled_row ) );
    }
}

int gr_video_ps4_present( SDL_Surface * src )
{
    OrbisKernelEvent event;
    int out = 0;
    int rc;

    if ( !src || !src->pixels || src->w < 1 || src->h < 1 )
        return 0;
    if ( !ps4_video_initialize() )
        return 0;

    ps4_blit_scaled( src, ( uint32_t * ) ps4_video.buffers[ ps4_video.current ] );
    rc = sceVideoOutSubmitFlip( ps4_video.handle, ps4_video.current,
                                ORBIS_VIDEO_OUT_FLIP_VSYNC,
                                ( int64_t ) ++ps4_video.flip_arg );
    if ( rc != 0 )
    {
        return 0;
    }
    rc = sceKernelWaitEqueue( ps4_video.flip_queue, &event, 1, &out, NULL );
    if ( rc != 0 )
    {
        return 0;
    }
    ps4_video.current = ( ps4_video.current + 1 ) % PS4_FB_COUNT;
    return 1;
}

int gr_video_ps4_present_rects( SDL_Surface * src, const SDL_Rect * rects, int count )
{
    ( void ) rects;
    ( void ) count;
    return gr_video_ps4_present( src );
}
