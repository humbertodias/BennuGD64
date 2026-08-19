/*
 * Wii pads for mod_joy, without SDL_OpenJoystick.
 *
 * Mapping matches bennugd-wii src/joystick/wii/SDL_sysjoystick.c so existing
 * Bennu games keep the same JOY_GETBUTTON / JOY_GETAXIS / JOY_GETHAT indices:
 *   0-3  Wiimote (+ Nunchuk / Classic)
 *   4-7  GameCube
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <gccore.h>
#include <ogc/pad.h>
#include <wiiuse/wpad.h>

#include <SDL3/SDL.h>

#include "libjoy_internal.h"
#include "libjoy_wii.h"

#define PI 3.14159265f

#define MAX_GC_JOYSTICKS  4
#define MAX_WII_JOYSTICKS 4
#define WII_JOY_COUNT     ( MAX_GC_JOYSTICKS + MAX_WII_JOYSTICKS )

#define MAX_GC_AXES     6
#define MAX_GC_BUTTONS  8
#define MAX_WII_AXES    9
#define MAX_WII_BUTTONS 20

#define AXIS_MIN -32768
#define AXIS_MAX  32767

static const u32 sdl_buttons_wii[] = {
    WPAD_BUTTON_A,
    WPAD_BUTTON_B,
    WPAD_BUTTON_1,
    WPAD_BUTTON_2,
    WPAD_BUTTON_MINUS,
    WPAD_BUTTON_PLUS,
    WPAD_BUTTON_HOME,
    WPAD_NUNCHUK_BUTTON_Z, /* 7 */
    WPAD_NUNCHUK_BUTTON_C, /* 8 */
    WPAD_CLASSIC_BUTTON_A, /* 9 */
    WPAD_CLASSIC_BUTTON_B,
    WPAD_CLASSIC_BUTTON_X,
    WPAD_CLASSIC_BUTTON_Y,
    WPAD_CLASSIC_BUTTON_FULL_L,
    WPAD_CLASSIC_BUTTON_FULL_R,
    WPAD_CLASSIC_BUTTON_ZL,
    WPAD_CLASSIC_BUTTON_ZR,
    WPAD_CLASSIC_BUTTON_MINUS,
    WPAD_CLASSIC_BUTTON_PLUS,
    WPAD_CLASSIC_BUTTON_HOME
};

static const u16 sdl_buttons_gc[] = {
    PAD_BUTTON_A,
    PAD_BUTTON_B,
    PAD_BUTTON_X,
    PAD_BUTTON_Y,
    PAD_TRIGGER_Z,
    PAD_TRIGGER_R,
    PAD_TRIGGER_L,
    PAD_BUTTON_START
};

typedef struct {
    int      nbuttons;
    int      naxes;
    int      nhats;
    uint8_t  buttons[ MAX_WII_BUTTONS ];
    int16_t  axes[ MAX_WII_AXES ];
    uint8_t  hat;
    int      accel_x;
    int      accel_y;
    int      accel_z;
    char     name[ 32 ];
} wii_pad_state;

static wii_pad_state pads[ WII_JOY_COUNT ];
static int wii_ready = 0;

static int wii_button_is_nunchuk( int idx )
{
    return idx == 7 || idx == 8;
}

static int wii_button_is_classic( int idx )
{
    return idx >= 9;
}

static int16_t axis8_to_sdl( int axis )
{
    if ( axis >= 128 ) return AXIS_MAX;
    if ( axis <= -128 ) return AXIS_MIN;
    return ( int16_t )( axis << 8 );
}

static int16_t wpad_orient( WPADData * data, int motion )
{
    float out;

    if ( motion == 0 )
        out = data->orient.pitch;
    else if ( motion == 1 )
        out = data->orient.roll;
    else
        out = data->orient.yaw;

    return ( int16_t )( ( out / 180.0f ) * 128.0f );
}

static int16_t wpad_stick( WPADData * data, u8 right, int axis )
{
    float mag = 0.0f;
    float ang = 0.0f;
    double val;

    switch ( data->exp.type )
    {
        case WPAD_EXP_NUNCHUK:
        case WPAD_EXP_GUITARHERO3:
            if ( right == 0 )
            {
                mag = data->exp.nunchuk.js.mag;
                ang = data->exp.nunchuk.js.ang;
            }
            break;

        case WPAD_EXP_CLASSIC:
            if ( right == 0 )
            {
                mag = data->exp.classic.ljs.mag;
                ang = data->exp.classic.ljs.ang;
            }
            else
            {
                mag = data->exp.classic.rjs.mag;
                ang = data->exp.classic.rjs.ang;
            }
            break;

        default:
            break;
    }

    if ( mag > 1.0f ) mag = 1.0f;
    else if ( mag < -1.0f ) mag = -1.0f;

    if ( axis == 0 )
        val = mag * sin( ( PI * ang ) / 180.0f );
    else
        val = mag * cos( ( PI * ang ) / 180.0f );

    return ( int16_t )( val * 128.0f );
}

static int16_t analog_trigger( float v )
{
    if ( v > 1.5f )
        return axis8_to_sdl( ( int ) v );
    if ( v < 0.0f ) v = 0.0f;
    if ( v > 1.0f ) v = 1.0f;
    return ( int16_t )( v * 32767.0f );
}

static int wii_valid( int joy )
{
    return wii_ready && joy >= 0 && joy < WII_JOY_COUNT;
}

static void update_wiimote( int chan )
{
    wii_pad_state * st = &pads[ chan ];
    u32 buttons, exp_type;
    WPADData * data;
    expansion_t exp;
    int i, axis, hat;
    int16_t value;

    memset( st->buttons, 0, sizeof( st->buttons ) );
    memset( st->axes, 0, sizeof( st->axes ) );
    st->hat = SDL_HAT_CENTERED;
    st->accel_x = st->accel_y = st->accel_z = 0;
    snprintf( st->name, sizeof( st->name ), "Wiimote %d", chan );

    buttons = WPAD_ButtonsHeld( chan );
    if ( WPAD_Probe( chan, &exp_type ) != 0 )
        exp_type = WPAD_EXP_NONE;

    data = WPAD_Data( chan );
    if ( !data )
        return;

    WPAD_Expansion( chan, &exp );

    if ( data->data_present )
    {
        st->accel_x = data->accel.x;
        st->accel_y = data->accel.y;
        st->accel_z = data->accel.z;
    }

    if ( exp_type == WPAD_EXP_CLASSIC )
    {
        hat = SDL_HAT_CENTERED;
        if ( buttons & WPAD_CLASSIC_BUTTON_UP )    hat |= SDL_HAT_UP;
        if ( buttons & WPAD_CLASSIC_BUTTON_DOWN )  hat |= SDL_HAT_DOWN;
        if ( buttons & WPAD_CLASSIC_BUTTON_LEFT )  hat |= SDL_HAT_LEFT;
        if ( buttons & WPAD_CLASSIC_BUTTON_RIGHT ) hat |= SDL_HAT_RIGHT;
        st->hat = ( uint8_t ) hat;
        snprintf( st->name, sizeof( st->name ), "Wiimote %d + Classic", chan );
    }
    else
    {
        /* Wiimote held sideways (NES-style), same as bennugd-wii. */
        hat = SDL_HAT_CENTERED;
        if ( buttons & WPAD_BUTTON_UP )    hat |= SDL_HAT_LEFT;
        if ( buttons & WPAD_BUTTON_DOWN )  hat |= SDL_HAT_RIGHT;
        if ( buttons & WPAD_BUTTON_LEFT )  hat |= SDL_HAT_DOWN;
        if ( buttons & WPAD_BUTTON_RIGHT ) hat |= SDL_HAT_UP;
        st->hat = ( uint8_t ) hat;
        if ( exp_type == WPAD_EXP_NUNCHUK )
            snprintf( st->name, sizeof( st->name ), "Wiimote %d + Nunchuk", chan );
    }

    for ( i = 0; i < ( int )( sizeof( sdl_buttons_wii ) / sizeof( sdl_buttons_wii[ 0 ] ) ); i++ )
    {
        if ( ( exp_type == WPAD_EXP_CLASSIC && wii_button_is_nunchuk( i ) ) ||
             ( exp_type == WPAD_EXP_NUNCHUK && wii_button_is_classic( i ) ) )
            continue;
        st->buttons[ i ] = ( buttons & sdl_buttons_wii[ i ] ) ? 1 : 0;
    }

    if ( exp_type == WPAD_EXP_CLASSIC )
    {
        axis = wpad_stick( data, 0, 0 );
        st->axes[ 0 ] = axis8_to_sdl( axis );
        axis = wpad_stick( data, 0, 1 );
        value = axis8_to_sdl( axis );
        st->axes[ 1 ] = ( int16_t )( -value );
        axis = wpad_stick( data, 1, 0 );
        st->axes[ 2 ] = ( int16_t )( axis << 8 );
        axis = wpad_stick( data, 1, 1 );
        st->axes[ 3 ] = ( int16_t )( -( axis << 8 ) );
        st->axes[ 4 ] = analog_trigger( exp.classic.r_shoulder );
        st->axes[ 5 ] = analog_trigger( exp.classic.l_shoulder );
    }
    else if ( exp_type == WPAD_EXP_NUNCHUK || exp_type == WPAD_EXP_GUITARHERO3 )
    {
        axis = wpad_stick( data, 0, 0 );
        st->axes[ 0 ] = axis8_to_sdl( axis );
        axis = wpad_stick( data, 0, 1 );
        value = axis8_to_sdl( axis );
        st->axes[ 1 ] = ( int16_t )( -value );
    }

    axis = wpad_orient( data, 0 );
    st->axes[ 6 ] = ( int16_t )( -( axis << 8 ) );
    axis = wpad_orient( data, 1 );
    st->axes[ 7 ] = ( int16_t )( axis << 8 );
    axis = wpad_orient( data, 2 );
    st->axes[ 8 ] = ( int16_t )( axis << 8 );
}

static void update_gamecube( int chan )
{
    wii_pad_state * st = &pads[ MAX_WII_JOYSTICKS + chan ];
    u16 buttons;
    int i, hat;

    memset( st->buttons, 0, sizeof( st->buttons ) );
    memset( st->axes, 0, sizeof( st->axes ) );
    st->hat = SDL_HAT_CENTERED;
    st->accel_x = st->accel_y = st->accel_z = 0;
    snprintf( st->name, sizeof( st->name ), "Gamecube %d", chan );

    buttons = PAD_ButtonsHeld( chan );

    hat = SDL_HAT_CENTERED;
    if ( buttons & PAD_BUTTON_UP )    hat |= SDL_HAT_UP;
    if ( buttons & PAD_BUTTON_DOWN )  hat |= SDL_HAT_DOWN;
    if ( buttons & PAD_BUTTON_LEFT )  hat |= SDL_HAT_LEFT;
    if ( buttons & PAD_BUTTON_RIGHT ) hat |= SDL_HAT_RIGHT;
    st->hat = ( uint8_t ) hat;

    for ( i = 0; i < ( int )( sizeof( sdl_buttons_gc ) / sizeof( sdl_buttons_gc[ 0 ] ) ); i++ )
        st->buttons[ i ] = ( buttons & sdl_buttons_gc[ i ] ) ? 1 : 0;

    st->axes[ 0 ] = ( int16_t )( PAD_StickX( chan ) << 8 );
    st->axes[ 1 ] = ( int16_t )( ( -PAD_StickY( chan ) ) << 8 );
    st->axes[ 2 ] = ( int16_t )( PAD_SubStickX( chan ) << 8 );
    st->axes[ 3 ] = ( int16_t )( PAD_SubStickY( chan ) << 8 );
    st->axes[ 4 ] = ( int16_t )( PAD_TriggerL( chan ) << 8 );
    st->axes[ 5 ] = ( int16_t )( PAD_TriggerR( chan ) << 8 );
}

void libjoy_wii_pump( void )
{
    int i;

    if ( !wii_ready )
        return;

    WPAD_ScanPads();
    PAD_ScanPads();

    for ( i = 0; i < MAX_WII_JOYSTICKS; i++ )
        update_wiimote( i );
    for ( i = 0; i < MAX_GC_JOYSTICKS; i++ )
        update_gamecube( i );

    {
        extern void bgdi_wii_handle_power( void );
        bgdi_wii_handle_power();
    }
}

void libjoy_wii_module_initialize( void )
{
    int i;

    PAD_Init();

    memset( pads, 0, sizeof( pads ) );
    for ( i = 0; i < MAX_WII_JOYSTICKS; i++ )
    {
        pads[ i ].nbuttons = MAX_WII_BUTTONS;
        pads[ i ].naxes    = MAX_WII_AXES;
        pads[ i ].nhats    = 1;
        snprintf( pads[ i ].name, sizeof( pads[ i ].name ), "Wiimote %d", i );
    }
    for ( i = 0; i < MAX_GC_JOYSTICKS; i++ )
    {
        pads[ MAX_WII_JOYSTICKS + i ].nbuttons = MAX_GC_BUTTONS;
        pads[ MAX_WII_JOYSTICKS + i ].naxes    = MAX_GC_AXES;
        pads[ MAX_WII_JOYSTICKS + i ].nhats    = 1;
        snprintf( pads[ MAX_WII_JOYSTICKS + i ].name, sizeof( pads[ MAX_WII_JOYSTICKS + i ].name ),
                  "Gamecube %d", i );
    }

    wii_ready = 1;
    _max_joys = WII_JOY_COUNT;
    libjoy_wii_pump();
}

void libjoy_wii_module_finalize( void )
{
    wii_ready = 0;
    _max_joys = 0;
}

int libjoy_wii_num( void )
{
    return wii_ready ? WII_JOY_COUNT : 0;
}

const char * libjoy_wii_name( int joy )
{
    if ( !wii_valid( joy ) )
        return "";
    return pads[ joy ].name;
}

int libjoy_wii_buttons( int joy )
{
    if ( !wii_valid( joy ) )
        return 0;
    return pads[ joy ].nbuttons;
}

int libjoy_wii_axes( int joy )
{
    if ( !wii_valid( joy ) )
        return 0;
    return pads[ joy ].naxes;
}

int libjoy_wii_hats( int joy )
{
    if ( !wii_valid( joy ) )
        return 0;
    return pads[ joy ].nhats;
}

int libjoy_wii_get_button( int joy, int button )
{
    if ( !wii_valid( joy ) || button < 0 || button >= pads[ joy ].nbuttons )
        return 0;
    return pads[ joy ].buttons[ button ];
}

int libjoy_wii_get_position( int joy, int axis )
{
    if ( !wii_valid( joy ) || axis < 0 || axis >= pads[ joy ].naxes )
        return 0;
    return pads[ joy ].axes[ axis ];
}

int libjoy_wii_get_hat( int joy, int hat )
{
    if ( !wii_valid( joy ) || hat != 0 )
        return 0;
    return pads[ joy ].hat;
}

int libjoy_wii_get_accel( int joy, int * x, int * y, int * z )
{
    if ( !wii_valid( joy ) || joy >= MAX_WII_JOYSTICKS )
        return -1;
    if ( x ) *x = pads[ joy ].accel_x;
    if ( y ) *y = pads[ joy ].accel_y;
    if ( z ) *z = pads[ joy ].accel_z;
    return 0;
}
