/*
 * PlayStation 5 DualSense → Bennu key(). Compiled only into the ps5-x86_64 build.
 *
 *   D-pad / left stick  → arrows
 *   Cross / R1          → C + Ctrl
 *   Circle / R2         → V + Alt
 *   Square / L1         → X + Space
 *   Triangle / L2       → B
 *   OPTIONS             → Enter
 *   TOUCH PAD           → Escape
 */

#include <string.h>

#include <SDL3/SDL.h>

#include "libkey_ps5.h"
#include "ps5_platform.h"

extern const bool * keystate;

#define PS5_ANA_CENTER  0x80
#define PS5_ANA_DEAD    0x40

static bool ps5_keystate[ SDL_SCANCODE_COUNT ];

static void set_sc( SDL_Scancode sc )
{
    if ( sc > 0 && sc < SDL_SCANCODE_COUNT )
        ps5_keystate[ sc ] = true;
}

static void apply_pad( const ScePadData * pad )
{
    unsigned ana_x, ana_y;
    uint32_t b;

    if ( !pad )
        return;
    b = pad->buttons;

    if ( b & SCE_PAD_BUTTON_UP )
        set_sc( SDL_SCANCODE_UP );
    if ( b & SCE_PAD_BUTTON_DOWN )
        set_sc( SDL_SCANCODE_DOWN );
    if ( b & SCE_PAD_BUTTON_LEFT )
        set_sc( SDL_SCANCODE_LEFT );
    if ( b & SCE_PAD_BUTTON_RIGHT )
        set_sc( SDL_SCANCODE_RIGHT );

    ana_x = ( unsigned ) pad->leftStick.x;
    ana_y = ( unsigned ) pad->leftStick.y;
    if ( ana_x < ( unsigned )( PS5_ANA_CENTER - PS5_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_LEFT );
    if ( ana_x > ( unsigned )( PS5_ANA_CENTER + PS5_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_RIGHT );
    if ( ana_y < ( unsigned )( PS5_ANA_CENTER - PS5_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_UP );
    if ( ana_y > ( unsigned )( PS5_ANA_CENTER + PS5_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_DOWN );

    if ( ( b & SCE_PAD_BUTTON_CROSS ) || ( b & SCE_PAD_BUTTON_R1 ) )
    {
        set_sc( SDL_SCANCODE_C );
        set_sc( SDL_SCANCODE_A );
        set_sc( SDL_SCANCODE_LCTRL );
    }
    if ( ( b & SCE_PAD_BUTTON_CIRCLE ) || ( b & SCE_PAD_BUTTON_R2 ) )
    {
        set_sc( SDL_SCANCODE_V );
        set_sc( SDL_SCANCODE_D );
        set_sc( SDL_SCANCODE_LALT );
    }
    if ( ( b & SCE_PAD_BUTTON_SQUARE ) || ( b & SCE_PAD_BUTTON_L1 ) )
    {
        set_sc( SDL_SCANCODE_X );
        set_sc( SDL_SCANCODE_S );
        set_sc( SDL_SCANCODE_SPACE );
    }
    if ( ( b & SCE_PAD_BUTTON_TRIANGLE ) || ( b & SCE_PAD_BUTTON_L2 ) )
        set_sc( SDL_SCANCODE_B );

    if ( b & SCE_PAD_BUTTON_OPTIONS )
        set_sc( SDL_SCANCODE_RETURN );
    if ( b & SCE_PAD_BUTTON_TOUCH_PAD )
        set_sc( SDL_SCANCODE_ESCAPE );
}

void libkey_ps5_after_init( SDL_Window * window )
{
    ( void ) window;
    libkey_ps5_after_events();
}

void libkey_ps5_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;
    ScePadData pad;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( ps5_keystate, 0, sizeof( ps5_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( ps5_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    ps5_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    if ( ps5_platform_pad_handle() >= 0 )
    {
        memset( &pad, 0, sizeof( pad ) );
        if ( ps5_platform_read_pad( &pad ) == 0 )
            apply_pad( &pad );
    }

    keystate = ps5_keystate;
}
