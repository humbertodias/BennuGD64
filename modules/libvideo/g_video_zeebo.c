/*
 * Zeebo video: VGA 640x480. SDL draws offscreen; each frame is copied into
 * the BREW device bitmap and IDISPLAY_Update shows it.
 */

#include <stdint.h>
#include <string.h>

#include "libvideo.h"
#include "g_video_zeebo.h"
#include "sdl3_compat.h"

#define AEECLSID_DIB 0x01001045u
#define ZEEBO_MODULE_BASE 0x00010000u
/* Appended after GetALSContext. Old emulators stop at version 1 and never see it. */
#define ZEEBO_PRESENT_SLOT 117

typedef uint32_t ( *zeebo_present_fn )( const void * pixels, uint32_t w, uint32_t h, uint32_t pitch );

typedef int ( *zeebo_getbmp_fn )( void * display, void ** out );
typedef int ( *zeebo_qi_fn )( void * bitmap, uint32_t clsid, void ** out );
typedef int ( *zeebo_update_fn )( void * display );

static void * zeebo_display;
static uint8_t * zeebo_pbmp;
static int zeebo_pitch;
static int zeebo_dw;
static int zeebo_dh;

void zeebo_set_display( void * display )
{
    zeebo_display = display;
}

static void ** zeebo_vt( void * obj )
{
    return *( void *** )obj;
}

static int zeebo_open_screen( void )
{
    zeebo_getbmp_fn getbmp;
    zeebo_qi_fn query;
    void * bitmap = 0;
    void * dib = 0;
    uint8_t * pbmp;
    uint16_t cx, cy;
    int16_t pitch;

    if ( zeebo_pbmp ) return 1;
    if ( !zeebo_display ) return 0;

    getbmp = ( zeebo_getbmp_fn )( uintptr_t )zeebo_vt( zeebo_display )[16];
    if ( getbmp( zeebo_display, &bitmap ) != 0 || !bitmap ) return 0;

    query = ( zeebo_qi_fn )( uintptr_t )zeebo_vt( bitmap )[2];
    if ( query( bitmap, AEECLSID_DIB, &dib ) != 0 || !dib ) return 0;

    pbmp = *( uint8_t ** )( ( char * )dib + 8 );
    cx = *( uint16_t * )( ( char * )dib + 20 );
    cy = *( uint16_t * )( ( char * )dib + 22 );
    pitch = *( int16_t * )( ( char * )dib + 24 );
    if ( !pbmp || cx == 0 || cy == 0 || pitch < ( int16_t )( cx * 2 ) ) return 0;

    zeebo_pbmp = pbmp;
    zeebo_pitch = pitch;
    zeebo_dw = cx;
    zeebo_dh = cy;
    return 1;
}

static void zeebo_scale2( const uint8_t * src, int src_pitch, int sw, int sh )
{
    int x, y;

    for ( y = 0; y < sh; y++ )
    {
        const uint16_t * srow = ( const uint16_t * )( src + y * src_pitch );
        uint16_t * d0 = ( uint16_t * )( zeebo_pbmp + ( y * 2 ) * zeebo_pitch );
        uint16_t * d1 = ( uint16_t * )( ( uint8_t * )d0 + zeebo_pitch );

        for ( x = 0; x < sw; x++ )
        {
            uint16_t p = srow[x];
            d0[x * 2] = p;
            d0[x * 2 + 1] = p;
        }
        memcpy( d1, d0, ( size_t )sw * 4u );
    }
}

static int zeebo_present_bulk( const uint8_t * pixels, int w, int h, int pitch )
{
    uint32_t ver = *( volatile uint32_t * )( uintptr_t )( ZEEBO_MODULE_BASE - 8 );
    void ** table;
    zeebo_present_fn fn;

    if ( ver < 2 ) return 0;
    table = *( void *** )( uintptr_t )( ZEEBO_MODULE_BASE - 4 );
    if ( !table ) return 0;
    fn = ( zeebo_present_fn )( uintptr_t )table[ZEEBO_PRESENT_SLOT];
    if ( !fn ) return 0;
    return fn( pixels, ( uint32_t )w, ( uint32_t )h, ( uint32_t )pitch ) == 0;
}

static void zeebo_blit_rgb565( const uint8_t * src, int src_pitch, int sw, int sh )
{
    int x, y;

    if ( sw == zeebo_dw && sh == zeebo_dh )
    {
        int row = sw * 2;
        for ( y = 0; y < sh; y++ )
            memcpy( zeebo_pbmp + y * zeebo_pitch, src + y * src_pitch, row );
        return;
    }

    if ( sw > 0 && sh > 0 && zeebo_dw == sw * 2 && zeebo_dh == sh * 2 )
    {
        zeebo_scale2( src, src_pitch, sw, sh );
        return;
    }

    for ( y = 0; y < zeebo_dh; y++ )
    {
        int sy = ( sh == 0 ) ? 0 : y * sh / zeebo_dh;
        const uint8_t * srow = src + sy * src_pitch;
        uint8_t * drow = zeebo_pbmp + y * zeebo_pitch;
        for ( x = 0; x < zeebo_dw; x++ )
        {
            int sx = ( sw == 0 ) ? 0 : x * sw / zeebo_dw;
            drow[x * 2] = srow[sx * 2];
            drow[x * 2 + 1] = srow[sx * 2 + 1];
        }
    }
}

void gr_video_zeebo_module_initialize( void )
{
    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "software" );
    SDL_SetHint( SDL_HINT_VIDEO_DRIVER, "dummy" );
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) SDL_InitSubSystem( SDL_INIT_VIDEO );
}

void gr_video_zeebo_adjust_window( int * width, int * height, Uint32 * window_flags )
{
    *width = 640;
    *height = 480;
    *window_flags |= SDL_WINDOW_FULLSCREEN;
}

void gr_video_zeebo_apply_mode( void )
{
    full_screen = 1;
}

void gr_video_zeebo_present( SDL_Surface * src )
{
    zeebo_update_fn update;
    SDL_Surface * frame = src;
    SDL_Surface * converted = NULL;
    int rgb565;

    if ( !src || !src->pixels || src->w <= 0 || src->h <= 0 ) return;

    rgb565 = ( src->format == SDL_PIXELFORMAT_RGB565 ) ||
             ( bennu_surface_bpp( src ) == 16 && bennu_surface_rmask( src ) == 0xF800 );
    if ( !rgb565 )
    {
        converted = SDL_ConvertSurface( src, SDL_PIXELFORMAT_RGB565 );
        if ( !converted || !converted->pixels )
        {
            if ( converted ) SDL_DestroySurface( converted );
            return;
        }
        frame = converted;
    }

    /* Bulk path first. Opening the device bitmap watches every later store. */
    if ( zeebo_present_bulk( frame->pixels, frame->w, frame->h, frame->pitch ) )
    {
        if ( converted ) SDL_DestroySurface( converted );
        return;
    }

    if ( !zeebo_open_screen() )
    {
        if ( converted ) SDL_DestroySurface( converted );
        return;
    }

    zeebo_blit_rgb565( frame->pixels, frame->pitch, frame->w, frame->h );
    if ( converted ) SDL_DestroySurface( converted );

    update = ( zeebo_update_fn )( uintptr_t )zeebo_vt( zeebo_display )[7];
    update( zeebo_display );
}
