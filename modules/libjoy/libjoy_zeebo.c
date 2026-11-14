/*
 * The Zeebo pad is an IHID device. SDL has no joystick driver here, so
 * Streets of Rage (and anything else that asks joy_*) saw zero pads.
 * One always-connected controller, read from the shell Zeebx already serves.
 *
 * Buttons follow the Xbox port, which matches SoRR's default map:
 *   0..3 face, 4 ZL, 5 ZR, 8 start, 9 back.
 * The d-pad is the hat. It also drives axes 0 and 1 while the stick is at
 * rest, because the keyboard in Zeebx is a d-pad and not a stick.
 */

#include <stdint.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "libjoy_zeebo.h"

#define AEECLSID_HID 0x0106c411u
#define UID_JOYSTICK 0x0106c3fdu

/* IHID and IHIDDevice slots, in the order zeebx-core publishes them. */
#define HID_CREATE_DEVICE 3
#define HID_GET_CONNECTED 7
#define DEV_GET_BUTTON 6
#define DEV_GET_POSITION 9

#define PAD_BUTTONS 12
#define PAD_INFO 16
#define AXIS_WORDS 25
#define AXIS_DEAD 8000

typedef int ( *zeebo_create_fn )( void * shell, uint32_t clsid, void ** out );
typedef int ( *zeebo_connected_fn )( void * hid, uint32_t type, uint32_t * handles, uint32_t len, uint32_t * needed );
typedef int ( *zeebo_device_fn )( void * hid, uint32_t handle, void ** out );
typedef int ( *zeebo_button_fn )( void * dev, uint32_t id, uint32_t * info );
typedef int ( *zeebo_position_fn )( void * dev, uint32_t * info );

extern void * zeebo_get_shell( void );

static void * zeebo_hid;
static void * zeebo_dev;
static int zeebo_ready;
static int zeebo_fresh;
static int zeebo_btn[PAD_INFO];
static int zeebo_axis[4];

static void ** zeebo_vt( void * obj )
{
    return *( void *** )obj;
}

static int zeebo_open( void )
{
    zeebo_create_fn create;
    zeebo_connected_fn connected;
    zeebo_device_fn make;
    void * shell;
    void * hid;
    void * dev = 0;
    uint32_t handles[2];
    uint32_t needed = 0;

    if ( zeebo_dev ) return 1;
    shell = zeebo_get_shell();
    if ( !shell ) return 0;
    if ( !zeebo_hid )
    {
        hid = 0;
        create = ( zeebo_create_fn )( uintptr_t )zeebo_vt( shell )[2];
        if ( create( shell, AEECLSID_HID, &hid ) != 0 || !hid ) return 0;
        zeebo_hid = hid;
    }
    connected = ( zeebo_connected_fn )( uintptr_t )zeebo_vt( zeebo_hid )[HID_GET_CONNECTED];
    handles[0] = handles[1] = 0;
    if ( connected( zeebo_hid, UID_JOYSTICK, handles, 2, &needed ) != 0 || needed == 0 || handles[0] == 0 )
        return 0;
    make = ( zeebo_device_fn )( uintptr_t )zeebo_vt( zeebo_hid )[HID_CREATE_DEVICE];
    if ( make( zeebo_hid, handles[0], &dev ) != 0 || !dev ) return 0;
    zeebo_dev = dev;
    zeebo_ready = 1;
    return 1;
}

static int zeebo_sdl_axis( uint32_t raw )
{
    int v = ( ( int )raw - 128 ) * 256;
    if ( v > 32767 ) v = 32767;
    if ( v < -32768 ) v = -32768;
    return v;
}

static void zeebo_refresh( void )
{
    zeebo_button_fn button;
    zeebo_position_fn position;
    uint32_t info[5];
    uint32_t axes[AXIS_WORDS];
    int i;

    for ( i = 0; i < PAD_INFO; i++ ) zeebo_btn[i] = 0;
    for ( i = 0; i < 4; i++ ) zeebo_axis[i] = 0;
    if ( !zeebo_open() ) return;

    button = ( zeebo_button_fn )( uintptr_t )zeebo_vt( zeebo_dev )[DEV_GET_BUTTON];
    for ( i = 0; i < PAD_INFO; i++ )
    {
        info[0] = info[1] = info[2] = info[3] = info[4] = 0;
        if ( button( zeebo_dev, ( uint32_t )i, info ) == 0 )
            zeebo_btn[i] = info[1] ? 1 : 0;
    }

    position = ( zeebo_position_fn )( uintptr_t )zeebo_vt( zeebo_dev )[DEV_GET_POSITION];
    for ( i = 0; i < AXIS_WORDS; i++ ) axes[i] = 128;
    axes[0] = 0;
    if ( position( zeebo_dev, axes ) == 0 )
    {
        /* X, Y, Z, RZ sit in words 1, 2, 3 and 6. */
        zeebo_axis[0] = zeebo_sdl_axis( axes[1] );
        zeebo_axis[1] = zeebo_sdl_axis( axes[2] );
        zeebo_axis[2] = zeebo_sdl_axis( axes[3] );
        zeebo_axis[3] = zeebo_sdl_axis( axes[6] );
    }

    if ( abs( zeebo_axis[0] ) < AXIS_DEAD && abs( zeebo_axis[1] ) < AXIS_DEAD )
    {
        if ( zeebo_btn[13] ) zeebo_axis[0] = -32767;
        if ( zeebo_btn[15] ) zeebo_axis[0] = 32767;
        if ( zeebo_btn[12] ) zeebo_axis[1] = -32767;
        if ( zeebo_btn[14] ) zeebo_axis[1] = 32767;
    }
}

static void zeebo_ensure( void )
{
    if ( !zeebo_fresh )
    {
        zeebo_refresh();
        zeebo_fresh = 1;
    }
}

static int zeebo_valid( int joy )
{
    return joy == 0 && zeebo_ready;
}

static int zeebo_face( int button )
{
    switch ( button )
    {
        case 0: return zeebo_btn[0];
        case 1: return zeebo_btn[1];
        case 2: return zeebo_btn[2];
        case 3: return zeebo_btn[3];
        case 4: return zeebo_btn[6];
        case 5: return zeebo_btn[4];
        case 8: return zeebo_btn[11];
        case 9: return zeebo_btn[9];
        case 10: return zeebo_btn[10];
        case 11: return zeebo_btn[8];
        default: return 0;
    }
}

void libjoy_zeebo_pump( void )
{
    zeebo_fresh = 0;
    zeebo_ensure();
}

void libjoy_zeebo_module_initialize( void )
{
    zeebo_fresh = 0;
    zeebo_open();
}

void libjoy_zeebo_module_finalize( void )
{
    zeebo_hid = 0;
    zeebo_dev = 0;
    zeebo_ready = 0;
    zeebo_fresh = 0;
}

int libjoy_zeebo_num( void )
{
    zeebo_ensure();
    return zeebo_ready ? 1 : 0;
}

const char * libjoy_zeebo_name( int joy )
{
    return zeebo_valid( joy ) ? "Zeebo Controller" : "";
}

int libjoy_zeebo_buttons( int joy )
{
    return zeebo_valid( joy ) ? PAD_BUTTONS : 0;
}

int libjoy_zeebo_axes( int joy )
{
    return zeebo_valid( joy ) ? 4 : 0;
}

int libjoy_zeebo_hats( int joy )
{
    return zeebo_valid( joy ) ? 1 : 0;
}

int libjoy_zeebo_get_button( int joy, int button )
{
    zeebo_ensure();
    if ( !zeebo_valid( joy ) ) return 0;
    return zeebo_face( button );
}

int libjoy_zeebo_get_position( int joy, int axis )
{
    zeebo_ensure();
    if ( !zeebo_valid( joy ) || axis < 0 || axis > 3 ) return 0;
    return zeebo_axis[axis];
}

int libjoy_zeebo_get_hat( int joy, int hat )
{
    int value = SDL_HAT_CENTERED;
    zeebo_ensure();
    if ( !zeebo_valid( joy ) || hat != 0 ) return value;
    if ( zeebo_btn[12] ) value |= SDL_HAT_UP;
    if ( zeebo_btn[14] ) value |= SDL_HAT_DOWN;
    if ( zeebo_btn[13] ) value |= SDL_HAT_LEFT;
    if ( zeebo_btn[15] ) value |= SDL_HAT_RIGHT;
    return value;
}

int libjoy_zeebo_get_accel( int joy, int * x, int * y, int * z )
{
    ( void )joy;
    ( void )x;
    ( void )y;
    ( void )z;
    return -1;
}
