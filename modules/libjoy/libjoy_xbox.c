#include <string.h>
#include <stdint.h>

#include <SDL3/SDL.h>
#include <windows.h>
#include <usbh_lib.h>
#include "xid_driver.h"

#include "libjoy_xbox.h"

#define XBOX_PADS CONFIG_XID_MAX_DEV
#define XBOX_DPAD_UP    0x0001
#define XBOX_DPAD_DOWN  0x0002
#define XBOX_DPAD_LEFT  0x0004
#define XBOX_DPAD_RIGHT 0x0008
#define XBOX_START      0x0010
#define XBOX_BACK       0x0020
#define XBOX_LSTICK     0x0040
#define XBOX_RSTICK     0x0080

static xid_gamepad_in xbox_pad[XBOX_PADS];
static int xbox_connected[XBOX_PADS];
static int xbox_usb_ready;

static int xbox_valid( int joy )
{
    return joy >= 0 && joy < XBOX_PADS && xbox_connected[joy];
}

static void xbox_store_pad( xid_dev_t * dev, xid_gamepad_in * report )
{
    xid_dev_t * list;
    int index = 0;

    if ( !dev || !report ) return;
    for ( list = usbh_xid_get_device_list(); list; list = list->next, index++ )
    {
        if ( list == dev && index < XBOX_PADS )
        {
            xbox_pad[index] = *report;
            xbox_connected[index] = 1;
            return;
        }
    }
}

static void xbox_read_callback( UTR_T * utr )
{
    xid_dev_t * xid;
    if ( !utr || utr->status != 0 || utr->xfer_len < sizeof( xid_gamepad_in ) )
        return;
    xid = ( xid_dev_t * )utr->context;
    xbox_store_pad( xid, ( xid_gamepad_in * )utr->buff );
    /* Re-queue the next interrupt transfer. */
    usbh_xid_read( xid, 0, ( void * )xbox_read_callback );
}

static void xbox_on_connect( xid_dev_t * xid, int param )
{
    ( void )param;
    if ( !xid ) return;
    if ( usbh_xid_get_type( xid ) == GAMECONTROLLER_S ||
         usbh_xid_get_type( xid ) == GAMECONTROLLER_DUKE ||
         ( usbh_xid_get_type( xid ) >> 8 ) == XID_TYPE_GAMECONTROLLER )
        usbh_xid_read( xid, 0, ( void * )xbox_read_callback );
}

static void xbox_on_disconnect( xid_dev_t * xid, int param )
{
    xid_dev_t * list;
    int index = 0;
    ( void )param;
    for ( list = usbh_xid_get_device_list(); list; list = list->next, index++ )
    {
        if ( list == xid && index < XBOX_PADS )
        {
            xbox_connected[index] = 0;
            memset( &xbox_pad[index], 0, sizeof( xbox_pad[index] ) );
            return;
        }
    }
}

void libjoy_xbox_pump( void )
{
    if ( xbox_usb_ready )
        usbh_pooling_hubs();
}

void libjoy_xbox_module_initialize( void )
{
    memset( xbox_pad, 0, sizeof( xbox_pad ) );
    memset( xbox_connected, 0, sizeof( xbox_connected ) );
    usbh_core_init();
    usbh_xid_init();
    usbh_install_xid_conn_callback( xbox_on_connect, xbox_on_disconnect );
    xbox_usb_ready = 1;
    libjoy_xbox_pump();
}

void libjoy_xbox_module_finalize( void )
{
    memset( xbox_connected, 0, sizeof( xbox_connected ) );
    xbox_usb_ready = 0;
}

int libjoy_xbox_num( void )
{
    int i;
    for ( i = XBOX_PADS - 1; i >= 0; i-- )
        if ( xbox_connected[i] ) return i + 1;
    return 0;
}

const char * libjoy_xbox_name( int joy )
{
    return xbox_valid( joy ) ? "Xbox Controller" : "";
}

int libjoy_xbox_buttons( int joy )
{
    return xbox_valid( joy ) ? 14 : 0;
}

int libjoy_xbox_axes( int joy )
{
    return xbox_valid( joy ) ? 6 : 0;
}

int libjoy_xbox_hats( int joy )
{
    return xbox_valid( joy ) ? 1 : 0;
}

int libjoy_xbox_get_button( int joy, int button )
{
    const xid_gamepad_in * p;
    if ( !xbox_valid( joy ) ) return 0;
    p = &xbox_pad[joy];
    switch ( button )
    {
        case 0: return p->a > 32;
        case 1: return p->b > 32;
        case 2: return p->x > 32;
        case 3: return p->y > 32;
        case 4: return p->white > 32;
        case 5: return p->black > 32;
        case 6: return p->l > 32;
        case 7: return p->r > 32;
        case 8: return ( p->dButtons & XBOX_START ) != 0;
        case 9: return ( p->dButtons & XBOX_BACK ) != 0;
        case 10: return ( p->dButtons & XBOX_LSTICK ) != 0;
        case 11: return ( p->dButtons & XBOX_RSTICK ) != 0;
        case 12: return ( p->dButtons & ( XBOX_DPAD_UP | XBOX_DPAD_DOWN |
                                          XBOX_DPAD_LEFT | XBOX_DPAD_RIGHT ) ) != 0;
        case 13: return p->l > 0 || p->r > 0;
        default: return 0;
    }
}

int libjoy_xbox_get_position( int joy, int axis )
{
    const xid_gamepad_in * p;
    if ( !xbox_valid( joy ) ) return 0;
    p = &xbox_pad[joy];
    switch ( axis )
    {
        case 0: return p->leftStickX;
        case 1: return -p->leftStickY;
        case 2: return p->rightStickX;
        case 3: return -p->rightStickY;
        case 4: return ( ( int )p->l * 257 ) - 32768;
        case 5: return ( ( int )p->r * 257 ) - 32768;
        default: return 0;
    }
}

int libjoy_xbox_get_hat( int joy, int hat )
{
    const xid_gamepad_in * p;
    int value = SDL_HAT_CENTERED;
    if ( !xbox_valid( joy ) || hat != 0 ) return value;
    p = &xbox_pad[joy];
    if ( p->dButtons & XBOX_DPAD_UP ) value |= SDL_HAT_UP;
    if ( p->dButtons & XBOX_DPAD_DOWN ) value |= SDL_HAT_DOWN;
    if ( p->dButtons & XBOX_DPAD_LEFT ) value |= SDL_HAT_LEFT;
    if ( p->dButtons & XBOX_DPAD_RIGHT ) value |= SDL_HAT_RIGHT;
    return value;
}

int libjoy_xbox_get_accel( int joy, int * x, int * y, int * z )
{
    ( void )joy;
    ( void )x;
    ( void )y;
    ( void )z;
    return -1;
}
