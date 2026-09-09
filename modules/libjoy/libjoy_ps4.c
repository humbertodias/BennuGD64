#include <stdint.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <orbis/Pad.h>

#include "ps4_platform.h"
#include "libjoy_ps4.h"

static OrbisPadData ps4_joy_state;
static int ps4_joy_connected;

static int ps4_joy_valid( int joy )
{
    return joy == 0 && ps4_platform_pad_handle() >= 0;
}

void libjoy_ps4_pump( void )
{
    memset( &ps4_joy_state, 0, sizeof( ps4_joy_state ) );
    ps4_joy_connected = ps4_platform_read_pad( &ps4_joy_state ) == 0;
}

void libjoy_ps4_module_initialize( void )
{
    ps4_platform_initialize();
    libjoy_ps4_pump();
}

void libjoy_ps4_module_finalize( void )
{
    ps4_joy_connected = 0;
}

int libjoy_ps4_num( void )
{
    return ps4_platform_pad_handle() >= 0 ? 1 : 0;
}

const char * libjoy_ps4_name( int joy )
{
    return ps4_joy_valid( joy ) ? "DualShock 4" : "";
}

int libjoy_ps4_buttons( int joy )
{
    return ps4_joy_valid( joy ) ? 12 : 0;
}

int libjoy_ps4_axes( int joy )
{
    return ps4_joy_valid( joy ) ? 4 : 0;
}

int libjoy_ps4_hats( int joy )
{
    return ps4_joy_valid( joy ) ? 1 : 0;
}

int libjoy_ps4_get_button( int joy, int button )
{
    static const uint32_t masks[] = {
        ORBIS_PAD_BUTTON_CROSS,
        ORBIS_PAD_BUTTON_CIRCLE,
        ORBIS_PAD_BUTTON_SQUARE,
        ORBIS_PAD_BUTTON_TRIANGLE,
        ORBIS_PAD_BUTTON_L1,
        ORBIS_PAD_BUTTON_R1,
        ORBIS_PAD_BUTTON_L2,
        ORBIS_PAD_BUTTON_R2,
        ORBIS_PAD_BUTTON_OPTIONS,
        ORBIS_PAD_BUTTON_TOUCH_PAD,
        ORBIS_PAD_BUTTON_L3,
        ORBIS_PAD_BUTTON_R3
    };

    if ( !ps4_joy_valid( joy ) || !ps4_joy_connected ||
         button < 0 || button >= ( int )( sizeof( masks ) / sizeof( masks[0] ) ) )
        return 0;
    return ( ps4_joy_state.buttons & masks[ button ] ) != 0;
}

static int ps4_axis( uint8_t value )
{
    return ( ( int ) value - 128 ) * 256;
}

int libjoy_ps4_get_position( int joy, int axis )
{
    if ( !ps4_joy_valid( joy ) || !ps4_joy_connected )
        return 0;
    switch ( axis )
    {
        case 0: return ps4_axis( ps4_joy_state.leftStick.x );
        case 1: return ps4_axis( ps4_joy_state.leftStick.y );
        case 2: return ps4_axis( ps4_joy_state.rightStick.x );
        case 3: return ps4_axis( ps4_joy_state.rightStick.y );
        default: return 0;
    }
}

int libjoy_ps4_get_hat( int joy, int hat )
{
    int value = SDL_HAT_CENTERED;

    if ( !ps4_joy_valid( joy ) || !ps4_joy_connected || hat != 0 )
        return value;
    if ( ps4_joy_state.buttons & ORBIS_PAD_BUTTON_UP ) value |= SDL_HAT_UP;
    if ( ps4_joy_state.buttons & ORBIS_PAD_BUTTON_DOWN ) value |= SDL_HAT_DOWN;
    if ( ps4_joy_state.buttons & ORBIS_PAD_BUTTON_LEFT ) value |= SDL_HAT_LEFT;
    if ( ps4_joy_state.buttons & ORBIS_PAD_BUTTON_RIGHT ) value |= SDL_HAT_RIGHT;
    return value;
}

int libjoy_ps4_get_accel( int joy, int * x, int * y, int * z )
{
    ( void ) joy;
    ( void ) x;
    ( void ) y;
    ( void ) z;
    return -1;
}
