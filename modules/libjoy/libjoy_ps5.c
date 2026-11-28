#include <stdint.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "ps5_platform.h"
#include "libjoy_ps5.h"

static ScePadData ps5_joy_state;
static int ps5_joy_connected;

static int ps5_joy_valid( int joy )
{
    return joy == 0 && ps5_platform_pad_handle() >= 0;
}

void libjoy_ps5_pump( void )
{
    memset( &ps5_joy_state, 0, sizeof( ps5_joy_state ) );
    ps5_joy_connected = ps5_platform_read_pad( &ps5_joy_state ) == 0;
}

void libjoy_ps5_module_initialize( void )
{
    ps5_platform_initialize();
    libjoy_ps5_pump();
}

void libjoy_ps5_module_finalize( void )
{
    ps5_joy_connected = 0;
}

int libjoy_ps5_num( void )
{
    return ps5_platform_pad_handle() >= 0 ? 1 : 0;
}

const char * libjoy_ps5_name( int joy )
{
    return ps5_joy_valid( joy ) ? "DualSense" : "";
}

int libjoy_ps5_buttons( int joy )
{
    return ps5_joy_valid( joy ) ? 12 : 0;
}

int libjoy_ps5_axes( int joy )
{
    return ps5_joy_valid( joy ) ? 4 : 0;
}

int libjoy_ps5_hats( int joy )
{
    return ps5_joy_valid( joy ) ? 1 : 0;
}

int libjoy_ps5_get_button( int joy, int button )
{
    static const uint32_t masks[] = {
        SCE_PAD_BUTTON_CROSS,
        SCE_PAD_BUTTON_CIRCLE,
        SCE_PAD_BUTTON_SQUARE,
        SCE_PAD_BUTTON_TRIANGLE,
        SCE_PAD_BUTTON_L1,
        SCE_PAD_BUTTON_R1,
        SCE_PAD_BUTTON_L2,
        SCE_PAD_BUTTON_R2,
        SCE_PAD_BUTTON_OPTIONS,
        SCE_PAD_BUTTON_TOUCH_PAD,
        SCE_PAD_BUTTON_L3,
        SCE_PAD_BUTTON_R3
    };

    if ( !ps5_joy_valid( joy ) || !ps5_joy_connected ||
         button < 0 || button >= ( int )( sizeof( masks ) / sizeof( masks[0] ) ) )
        return 0;
    return ( ps5_joy_state.buttons & masks[ button ] ) != 0;
}

static int ps5_axis( uint8_t value )
{
    return ( ( int ) value - 128 ) * 256;
}

int libjoy_ps5_get_position( int joy, int axis )
{
    if ( !ps5_joy_valid( joy ) || !ps5_joy_connected )
        return 0;
    switch ( axis )
    {
        case 0: return ps5_axis( ps5_joy_state.leftStick.x );
        case 1: return ps5_axis( ps5_joy_state.leftStick.y );
        case 2: return ps5_axis( ps5_joy_state.rightStick.x );
        case 3: return ps5_axis( ps5_joy_state.rightStick.y );
        default: return 0;
    }
}

int libjoy_ps5_get_hat( int joy, int hat )
{
    int value = SDL_HAT_CENTERED;

    if ( !ps5_joy_valid( joy ) || !ps5_joy_connected || hat != 0 )
        return value;
    if ( ps5_joy_state.buttons & SCE_PAD_BUTTON_UP ) value |= SDL_HAT_UP;
    if ( ps5_joy_state.buttons & SCE_PAD_BUTTON_DOWN ) value |= SDL_HAT_DOWN;
    if ( ps5_joy_state.buttons & SCE_PAD_BUTTON_LEFT ) value |= SDL_HAT_LEFT;
    if ( ps5_joy_state.buttons & SCE_PAD_BUTTON_RIGHT ) value |= SDL_HAT_RIGHT;
    return value;
}

int libjoy_ps5_get_accel( int joy, int * x, int * y, int * z )
{
    ( void ) joy;
    ( void ) x;
    ( void ) y;
    ( void ) z;
    return -1;
}
