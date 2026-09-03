/*
 * PlayStation 4 DualShock → Bennu key(). Compiled only into the ps4-x86_64 build.
 *
 *   D-pad / left stick  → arrows
 *   Cross / R1          → C + Ctrl
 *   Circle / R2         → V + Alt
 *   Square / L1         → X + Space
 *   Triangle / L2       → B
 *   OPTIONS             → Enter
 *   TOUCH PAD           → Escape
 *
 * Joystick (JOY_*): SDL pad as joy 0 when available. OS_LINUX in misc_ps4.c
 * makes SoRR use that pad as P1 when joystick mode is on.
 */

#include <string.h>

#include <orbis/Pad.h>
#include <orbis/UserService.h>
#include <SDL3/SDL.h>

#include "libkey_ps4.h"

extern const bool * keystate;

#define PS4_ANA_CENTER  0x80
#define PS4_ANA_DEAD    0x40

static bool ps4_keystate[ SDL_SCANCODE_COUNT ];
static int ps4_pad_handle = -1;

static void set_sc( SDL_Scancode sc )
{
    if ( sc > 0 && sc < SDL_SCANCODE_COUNT )
        ps4_keystate[ sc ] = true;
}

static void apply_pad( const OrbisPadData * pad )
{
    unsigned ana_x, ana_y;
    uint32_t b;

    if ( !pad )
        return;
    b = pad->buttons;

    if ( b & ORBIS_PAD_BUTTON_UP )
        set_sc( SDL_SCANCODE_UP );
    if ( b & ORBIS_PAD_BUTTON_DOWN )
        set_sc( SDL_SCANCODE_DOWN );
    if ( b & ORBIS_PAD_BUTTON_LEFT )
        set_sc( SDL_SCANCODE_LEFT );
    if ( b & ORBIS_PAD_BUTTON_RIGHT )
        set_sc( SDL_SCANCODE_RIGHT );

    ana_x = ( unsigned ) pad->leftStick.x;
    ana_y = ( unsigned ) pad->leftStick.y;
    if ( ana_x < ( unsigned )( PS4_ANA_CENTER - PS4_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_LEFT );
    if ( ana_x > ( unsigned )( PS4_ANA_CENTER + PS4_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_RIGHT );
    if ( ana_y < ( unsigned )( PS4_ANA_CENTER - PS4_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_UP );
    if ( ana_y > ( unsigned )( PS4_ANA_CENTER + PS4_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_DOWN );

    if ( ( b & ORBIS_PAD_BUTTON_CROSS ) || ( b & ORBIS_PAD_BUTTON_R1 ) )
    {
        set_sc( SDL_SCANCODE_C );
        set_sc( SDL_SCANCODE_A );
        set_sc( SDL_SCANCODE_LCTRL );
    }
    if ( ( b & ORBIS_PAD_BUTTON_CIRCLE ) || ( b & ORBIS_PAD_BUTTON_R2 ) )
    {
        set_sc( SDL_SCANCODE_V );
        set_sc( SDL_SCANCODE_D );
        set_sc( SDL_SCANCODE_LALT );
    }
    if ( ( b & ORBIS_PAD_BUTTON_SQUARE ) || ( b & ORBIS_PAD_BUTTON_L1 ) )
    {
        set_sc( SDL_SCANCODE_X );
        set_sc( SDL_SCANCODE_S );
        set_sc( SDL_SCANCODE_SPACE );
    }
    if ( ( b & ORBIS_PAD_BUTTON_TRIANGLE ) || ( b & ORBIS_PAD_BUTTON_L2 ) )
        set_sc( SDL_SCANCODE_B );

    if ( b & ORBIS_PAD_BUTTON_OPTIONS )
        set_sc( SDL_SCANCODE_RETURN );
    if ( b & ORBIS_PAD_BUTTON_TOUCH_PAD )
        set_sc( SDL_SCANCODE_ESCAPE );
}

void libkey_ps4_after_init( SDL_Window * window )
{
    int32_t user = 0;

    ( void ) window;
    sceUserServiceInitialize( NULL );
    scePadInit();
    if ( sceUserServiceGetInitialUser( &user ) != 0 )
        user = 0x1;
    ps4_pad_handle = scePadOpen( user, ORBIS_PAD_PORT_TYPE_STANDARD, 0, NULL );
    libkey_ps4_after_events();
}

void libkey_ps4_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;
    OrbisPadData pad;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( ps4_keystate, 0, sizeof( ps4_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( ps4_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    ps4_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    if ( ps4_pad_handle >= 0 )
    {
        memset( &pad, 0, sizeof( pad ) );
        if ( scePadReadState( ps4_pad_handle, &pad ) == 0 )
            apply_pad( &pad );
    }

    keystate = ps4_keystate;
}
