#include <string.h>

#include <SDL3/SDL.h>
#include <input/input.h>
#include <usb/usbmain.h>

#include "libjoy_xbox360.h"

#define XBOX360_PADS 4

static struct controller_data_s xbox360_pad[XBOX360_PADS];
static int xbox360_connected[XBOX360_PADS];

static int xbox360_valid( int joy )
{
    return joy >= 0 && joy < XBOX360_PADS && xbox360_connected[joy];
}

void libjoy_xbox360_pump( void )
{
    int i;
    usb_do_poll();
    for ( i = 0; i < XBOX360_PADS; i++ )
    {
        memset( &xbox360_pad[i], 0, sizeof( xbox360_pad[i] ) );
        xbox360_connected[i] = get_controller_data( &xbox360_pad[i], i ) != 0;
    }
}

void libjoy_xbox360_module_initialize( void )
{
    libjoy_xbox360_pump();
}

void libjoy_xbox360_module_finalize( void )
{
    memset( xbox360_connected, 0, sizeof( xbox360_connected ) );
}

int libjoy_xbox360_num( void )
{
    int i;
    for ( i = XBOX360_PADS - 1; i >= 0; i-- )
        if ( xbox360_connected[i] ) return i + 1;
    return 0;
}

const char * libjoy_xbox360_name( int joy )
{
    return xbox360_valid( joy ) ? "Xbox 360 Controller" : "";
}

int libjoy_xbox360_buttons( int joy )
{
    return xbox360_valid( joy ) ? 15 : 0;
}

int libjoy_xbox360_axes( int joy )
{
    return xbox360_valid( joy ) ? 6 : 0;
}

int libjoy_xbox360_hats( int joy )
{
    return xbox360_valid( joy ) ? 1 : 0;
}

int libjoy_xbox360_get_button( int joy, int button )
{
    const struct controller_data_s * p;
    if ( !xbox360_valid( joy ) ) return 0;
    p = &xbox360_pad[joy];
    switch ( button )
    {
        case 0: return p->a;
        case 1: return p->b;
        case 2: return p->x;
        case 3: return p->y;
        case 4: return p->lb;
        case 5: return p->rb;
        case 6: return p->lt > 127;
        case 7: return p->rt > 127;
        case 8: return p->start;
        case 9: return p->back;
        case 10: return p->logo;
        case 11: return p->s1_z;
        case 12: return p->s2_z;
        case 13: return p->left || p->right || p->up || p->down;
        case 14: return p->lt || p->rt;
        default: return 0;
    }
}

int libjoy_xbox360_get_position( int joy, int axis )
{
    const struct controller_data_s * p;
    if ( !xbox360_valid( joy ) ) return 0;
    p = &xbox360_pad[joy];
    switch ( axis )
    {
        case 0: return p->s1_x;
        case 1: return -p->s1_y;
        case 2: return p->s2_x;
        case 3: return -p->s2_y;
        case 4: return ( ( int )p->lt * 257 ) - 32768;
        case 5: return ( ( int )p->rt * 257 ) - 32768;
        default: return 0;
    }
}

int libjoy_xbox360_get_hat( int joy, int hat )
{
    const struct controller_data_s * p;
    int value = SDL_HAT_CENTERED;
    if ( !xbox360_valid( joy ) || hat != 0 ) return value;
    p = &xbox360_pad[joy];
    if ( p->up ) value |= SDL_HAT_UP;
    if ( p->down ) value |= SDL_HAT_DOWN;
    if ( p->left ) value |= SDL_HAT_LEFT;
    if ( p->right ) value |= SDL_HAT_RIGHT;
    return value;
}

int libjoy_xbox360_get_accel( int joy, int * x, int * y, int * z )
{
    ( void )joy;
    ( void )x;
    ( void )y;
    ( void )z;
    return -1;
}
