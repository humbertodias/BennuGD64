#include <string.h>

#include <input/input.h>
#include <usb/usbmain.h>

#include "libkey_xbox360.h"

extern const bool * keystate;

static bool xbox360_keys[SDL_SCANCODE_COUNT];

static void xbox360_set( SDL_Scancode code )
{
    if ( code > 0 && code < SDL_SCANCODE_COUNT ) xbox360_keys[code] = true;
}

void libkey_xbox360_after_events( void )
{
    struct controller_data_s pad;

    memset( xbox360_keys, 0, sizeof( xbox360_keys ) );
    memset( &pad, 0, sizeof( pad ) );
    usb_do_poll();
    if ( get_controller_data( &pad, 0 ) )
    {
        if ( pad.up || pad.s1_y > 16384 ) xbox360_set( SDL_SCANCODE_UP );
        if ( pad.down || pad.s1_y < -16384 ) xbox360_set( SDL_SCANCODE_DOWN );
        if ( pad.left || pad.s1_x < -16384 ) xbox360_set( SDL_SCANCODE_LEFT );
        if ( pad.right || pad.s1_x > 16384 ) xbox360_set( SDL_SCANCODE_RIGHT );
        if ( pad.a || pad.rb )
        {
            xbox360_set( SDL_SCANCODE_A );
            xbox360_set( SDL_SCANCODE_C );
            xbox360_set( SDL_SCANCODE_LCTRL );
        }
        if ( pad.b || pad.rt > 127 )
        {
            xbox360_set( SDL_SCANCODE_D );
            xbox360_set( SDL_SCANCODE_V );
            xbox360_set( SDL_SCANCODE_LALT );
        }
        if ( pad.x || pad.lb )
        {
            xbox360_set( SDL_SCANCODE_S );
            xbox360_set( SDL_SCANCODE_X );
            xbox360_set( SDL_SCANCODE_SPACE );
        }
        if ( pad.y || pad.lt > 127 ) xbox360_set( SDL_SCANCODE_B );
        if ( pad.start ) xbox360_set( SDL_SCANCODE_RETURN );
        if ( pad.back ) xbox360_set( SDL_SCANCODE_ESCAPE );
    }
    keystate = xbox360_keys;
}

void libkey_xbox360_after_init( SDL_Window * window )
{
    ( void )window;
    libkey_xbox360_after_events();
}
