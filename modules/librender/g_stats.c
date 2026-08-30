/*
 * FPS + heap overlay. Off until F12, ` or O (keyboard), or Select+L1+R1 on PS2.
 * Drawn after the frame; restore_type/dump_type default to partial dirty rects,
 * so the present path must upload the overlay region (or the whole frame).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "librender.h"
#include "g_stats.h"

#if defined(TARGET_PS2)
#include <kernel.h>
#endif
#if defined(TARGET_PS2) || defined(TARGET_PSP) || defined(TARGET_VITA) || defined(__linux__)
#include <malloc.h>
#elif defined(__APPLE__)
#include <malloc/malloc.h>
#endif

#define STATS_SCALE  2
#define STATS_X      4
#define STATS_Y      4

static int stats_on;
static int stats_was_on;
static int hotkey_was_down;
static int force_present;

/* 5x7 glyphs, bit 4 is the leftmost pixel. */
static const char glyph_map[] = "0123456789FPSMB/:.";
static const uint8_t glyph_bits[][7] = {
    { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e }, /* 0 */
    { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e }, /* 1 */
    { 0x0e, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1f }, /* 2 */
    { 0x0e, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0e }, /* 3 */
    { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 }, /* 4 */
    { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e }, /* 5 */
    { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e }, /* 6 */
    { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 }, /* 7 */
    { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e }, /* 8 */
    { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c }, /* 9 */
    { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 }, /* F */
    { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 }, /* P */
    { 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e }, /* S */
    { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 }, /* M */
    { 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e }, /* B */
    { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 }, /* / */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 }, /* : */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 }, /* . */
};

void gr_stats_toggle( void )
{
    stats_on = !stats_on;
}

int gr_stats_force_present( void )
{
    return force_present;
}

static void stats_boot( void )
{
    static int once;
    const char * e;

    if ( once )
        return;
    once = 1;
    e = getenv( "BGD_STATS" );
    if ( e && e[0] && e[0] != '0' )
        stats_on = 1;
}

static uint32_t mem_used( void )
{
#if defined(TARGET_PS2) || defined(TARGET_PSP) || defined(TARGET_VITA) || defined(__linux__)
    struct mallinfo mi = mallinfo();
    return ( uint32_t ) mi.uordblks;
#elif defined(__APPLE__)
    malloc_statistics_t t;
    memset( &t, 0, sizeof( t ) );
    malloc_zone_statistics( NULL, &t );
    return ( uint32_t ) t.size_in_use;
#else
    return 0;
#endif
}

static uint32_t mem_total( void )
{
#if defined(TARGET_PS2)
    {
        s32 n = GetMemorySize();
        if ( n > 0 )
            return ( uint32_t ) n;
        return 32u * 1024u * 1024u;
    }
#elif defined(TARGET_PSP)
    return 32u * 1024u * 1024u;
#elif defined(TARGET_VITA)
    return 512u * 1024u * 1024u;
#elif defined(TARGET_WII)
    return 24u * 1024u * 1024u;
#else
    /* SDL, not windows.h: Win32 OBJECTID collides with librender.h. */
    {
        int mb = SDL_GetSystemRAM();
        if ( mb > 0 )
            return ( uint32_t ) mb * 1024u * 1024u;
    }
    return 0;
#endif
}

static void put_raw( GRAPH * dest, int x, int y, uint32_t color )
{
    uint8_t * p;

    if ( !dest || !dest->data )
        return;
    if ( x < 0 || y < 0 || x >= ( int ) dest->width || y >= ( int ) dest->height )
        return;
    p = ( uint8_t * ) dest->data + dest->pitch * y;
    switch ( dest->format->depth )
    {
        case 8:
            p[ x ] = ( uint8_t ) color;
            break;
        case 16:
            ( ( uint16_t * ) p )[ x ] = ( uint16_t ) color;
            break;
        case 32:
            ( ( uint32_t * ) p )[ x ] = color;
            break;
        default:
            break;
    }
}

static const uint8_t * glyph_for( char c )
{
    const char * p = strchr( glyph_map, c );
    if ( !p )
        return NULL;
    return glyph_bits[ p - glyph_map ];
}

static void draw_char( GRAPH * dest, int x, int y, char c, uint32_t fg, uint32_t bg )
{
    const uint8_t * g;
    int row, col, sy, sx;

    g = glyph_for( c );
    if ( !g )
        return;
    for ( row = 0 ; row < 7 ; row++ )
    {
        for ( col = 0 ; col < 5 ; col++ )
        {
            uint32_t color = ( g[ row ] & ( 0x10 >> col ) ) ? fg : bg;
            for ( sy = 0 ; sy < STATS_SCALE ; sy++ )
                for ( sx = 0 ; sx < STATS_SCALE ; sx++ )
                    put_raw( dest, x + col * STATS_SCALE + sx, y + row * STATS_SCALE + sy, color );
        }
    }
}

static uint32_t fg_color( GRAPH * dest )
{
    if ( !dest || !dest->format )
        return 0xFFFF;
    if ( dest->format->depth == 8 )
        return 255;
    return ( uint32_t ) _rgb( dest->format, 255, 255, 255 );
}

static uint32_t bg_color( GRAPH * dest )
{
    if ( !dest || !dest->format )
        return 0;
    if ( dest->format->depth == 8 )
        return 0;
    return ( uint32_t ) _rgb( dest->format, 0, 0, 0 );
}

static void restore_box( GRAPH * dest, int x2, int y2 )
{
    REGION r;

    r.x = 0;
    r.y = 0;
    r.x2 = x2;
    r.y2 = y2;
    if ( r.x2 >= ( int ) dest->width )
        r.x2 = ( int ) dest->width - 1;
    if ( r.y2 >= ( int ) dest->height )
        r.y2 = ( int ) dest->height - 1;
    if ( r.x2 < r.x || r.y2 < r.y )
        return;

    if ( background && !( background->info_flags & GI_CLEAN ) )
        gr_blit( dest, &r, 0, 0, B_NOCOLORKEY, background );
    else
        gr_clear_region( dest, &r );
}

void gr_stats_poll( void )
{
    const bool * keys;
    int down;

    stats_boot();

    keys = SDL_GetKeyboardState( NULL );
    if ( !keys )
        return;
    /* F12/` are often media/layout keys on Mac. O is a real key. */
    down = ( keys[ SDL_SCANCODE_F12 ] ||
             keys[ SDL_SCANCODE_GRAVE ] ||
             keys[ SDL_SCANCODE_O ] ) ? 1 : 0;
    if ( down && !hotkey_was_down )
        gr_stats_toggle();
    hotkey_was_down = down;
}

void gr_stats_draw( GRAPH * dest )
{
    char line[ 48 ];
    uint32_t used, total, fg, bg;
    int fps, i, x, y, box_x2, box_y2;

    stats_boot();
    force_present = 0;

    if ( !dest )
        return;
    if ( !stats_on && !stats_was_on )
        return;

    box_x2 = STATS_X + 22 * ( 5 * STATS_SCALE + STATS_SCALE );
    box_y2 = STATS_Y + 7 * STATS_SCALE + 2;
    restore_box( dest, box_x2, box_y2 );
    force_present = 1;

    if ( stats_on )
    {
        fps = ( int ) GLODWORD( librender, FPS );
        used = mem_used() / ( 1024u * 1024u );
        total = mem_total() / ( 1024u * 1024u );
        snprintf( line, sizeof( line ), "FPS %d  %u/%uM", fps, used, total );

        fg = fg_color( dest );
        bg = bg_color( dest );
        x = STATS_X;
        y = STATS_Y;
        for ( i = 0 ; line[ i ] ; i++ )
        {
            if ( line[ i ] != ' ' )
                draw_char( dest, x, y, line[ i ], fg, bg );
            x += 5 * STATS_SCALE + STATS_SCALE;
        }
    }

    stats_was_on = stats_on;
}
