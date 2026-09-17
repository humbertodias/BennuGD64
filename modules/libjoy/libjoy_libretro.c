#include "libjoy_libretro.h"
#include "libretro.h"

#define LIBRETRO_MAX_JOYS 16

short int libretro_input_state_cb( unsigned port, unsigned device, unsigned index, unsigned id );

static const unsigned joy_buttons[] =
{
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_X,
    RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
    RETRO_DEVICE_ID_JOYPAD_SELECT,
    RETRO_DEVICE_ID_JOYPAD_START,
    RETRO_DEVICE_ID_JOYPAD_L3,
    RETRO_DEVICE_ID_JOYPAD_R3,
    RETRO_DEVICE_ID_JOYPAD_L2,
    RETRO_DEVICE_ID_JOYPAD_R2
};

void libjoy_libretro_module_initialize( void )
{
}

void libjoy_libretro_module_finalize( void )
{
}

int libjoy_libretro_num( void )
{
    return LIBRETRO_MAX_JOYS;
}

const char * libjoy_libretro_name( int joy )
{
    ( void ) joy;
    return "Retropad";
}

int libjoy_libretro_buttons( int joy )
{
    ( void ) joy;
    return ( int ) ( sizeof( joy_buttons ) / sizeof( joy_buttons[0] ) );
}

int libjoy_libretro_axes( int joy )
{
    ( void ) joy;
    return 4;
}

int libjoy_libretro_hats( int joy )
{
    ( void ) joy;
    return 1;
}

int libjoy_libretro_get_button( int joy, int button )
{
    if ( joy < 0 || joy >= LIBRETRO_MAX_JOYS )
        return 0;
    if ( button < 0 || button >= ( int ) ( sizeof( joy_buttons ) / sizeof( joy_buttons[0] ) ) )
        return 0;
    return libretro_input_state_cb( ( unsigned ) joy, RETRO_DEVICE_JOYPAD, 0, joy_buttons[button] ) ? 1 : 0;
}

int libjoy_libretro_get_position( int joy, int axis )
{
    unsigned index;
    unsigned id;
    if ( joy < 0 || joy >= LIBRETRO_MAX_JOYS )
        return 0;
    if ( axis < 0 || axis > 3 )
        return 0;
    index = ( axis < 2 ) ? RETRO_DEVICE_INDEX_ANALOG_LEFT : RETRO_DEVICE_INDEX_ANALOG_RIGHT;
    id = ( axis & 1 ) ? RETRO_DEVICE_ID_ANALOG_Y : RETRO_DEVICE_ID_ANALOG_X;
    return ( int ) libretro_input_state_cb( ( unsigned ) joy, RETRO_DEVICE_ANALOG, index, id );
}

int libjoy_libretro_get_hat( int joy, int hat )
{
    int result = 0;
    if ( joy < 0 || joy >= LIBRETRO_MAX_JOYS || hat != 0 )
        return 0;
    if ( libretro_input_state_cb( ( unsigned ) joy, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP ) )
        result |= 0x01;
    if ( libretro_input_state_cb( ( unsigned ) joy, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT ) )
        result |= 0x02;
    if ( libretro_input_state_cb( ( unsigned ) joy, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN ) )
        result |= 0x04;
    if ( libretro_input_state_cb( ( unsigned ) joy, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT ) )
        result |= 0x08;
    return result;
}

int libjoy_libretro_get_accel( int joy, int * x, int * y, int * z )
{
    ( void ) joy;
    if ( x ) *x = 0;
    if ( y ) *y = 0;
    if ( z ) *z = 0;
    return -1;
}
