/*
 * PlayStation 3 DualShock + USB keyboard → Bennu key(). Compiled only
 * into the ps3-ppu build.
 *
 * SoRR menus use key(_up)/key(_enter). In-game P1 keyboard layout is
 * arrows plus X/C/V/B (and Ctrl/Alt/Space). SDL on PS3 often only
 * reports arrows and Enter; the rest comes from ioPad / ioKb.
 *
 *   D-pad / left stick  → arrows
 *   Cross / R1          → C + Ctrl
 *   Circle / R2         → V + Alt
 *   Square / L1         → X + Space
 *   Triangle / L2       → B
 *   START               → Enter
 *   SELECT              → Escape
 *
 * USB keyboard: ioKb RAW HID codes (same numbering as SDL scancodes).
 * Phantom SDL Escape is cleared (hello.prg).
 *
 * Joystick (JOY_*): SDL pad as joy 0. OS_LINUX in misc_ps3.c makes
 * SoRR use that pad as P1 when joystick mode is on.
 */

#include <string.h>

#include <io/kb.h>
#include <io/pad.h>
#include <SDL3/SDL.h>

#include "libkey_ps3.h"

extern const bool * keystate;

#define PS3_ANA_CENTER  0x80
#define PS3_ANA_DEAD    0x40

static bool ps3_keystate[ SDL_SCANCODE_COUNT ];
static padData ps3_last_pad;
static int ps3_have_pad;
static int ps3_kb_ok;

static void set_sc( SDL_Scancode sc )
{
    if ( sc > 0 && sc < SDL_SCANCODE_COUNT )
        ps3_keystate[ sc ] = true;
}

static void apply_pad( const padData * pad )
{
    unsigned ana_x, ana_y;

    if ( pad->BTN_UP )
        set_sc( SDL_SCANCODE_UP );
    if ( pad->BTN_DOWN )
        set_sc( SDL_SCANCODE_DOWN );
    if ( pad->BTN_LEFT )
        set_sc( SDL_SCANCODE_LEFT );
    if ( pad->BTN_RIGHT )
        set_sc( SDL_SCANCODE_RIGHT );

    ana_x = ( unsigned ) pad->ANA_L_H & 0xFF;
    ana_y = ( unsigned ) pad->ANA_L_V & 0xFF;
    if ( ana_x && ana_x < ( unsigned )( PS3_ANA_CENTER - PS3_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_LEFT );
    if ( ana_x > ( unsigned )( PS3_ANA_CENTER + PS3_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_RIGHT );
    if ( ana_y && ana_y < ( unsigned )( PS3_ANA_CENTER - PS3_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_UP );
    if ( ana_y > ( unsigned )( PS3_ANA_CENTER + PS3_ANA_DEAD ) )
        set_sc( SDL_SCANCODE_DOWN );

    /* Same face-button map as the PS2 port (SoRR X/C/V/B + Ctrl/Alt/Space). */
    if ( pad->BTN_CROSS || pad->BTN_R1 )
    {
        set_sc( SDL_SCANCODE_C );
        set_sc( SDL_SCANCODE_A );
        set_sc( SDL_SCANCODE_LCTRL );
    }
    if ( pad->BTN_CIRCLE || pad->BTN_R2 )
    {
        set_sc( SDL_SCANCODE_V );
        set_sc( SDL_SCANCODE_D );
        set_sc( SDL_SCANCODE_LALT );
    }
    if ( pad->BTN_SQUARE || pad->BTN_L1 )
    {
        set_sc( SDL_SCANCODE_X );
        set_sc( SDL_SCANCODE_S );
        set_sc( SDL_SCANCODE_SPACE );
    }
    if ( pad->BTN_TRIANGLE || pad->BTN_L2 )
        set_sc( SDL_SCANCODE_B );

    if ( pad->BTN_START )
        set_sc( SDL_SCANCODE_RETURN );
    if ( pad->BTN_SELECT )
        set_sc( SDL_SCANCODE_ESCAPE );
}

static void apply_kb( const KbData * kb )
{
    int i;
    u16 raw;

    if ( kb->mkey._KbMkeyU._KbMkeyS.l_ctrl || kb->mkey._KbMkeyU._KbMkeyS.r_ctrl )
        set_sc( SDL_SCANCODE_LCTRL );
    if ( kb->mkey._KbMkeyU._KbMkeyS.l_alt || kb->mkey._KbMkeyU._KbMkeyS.r_alt )
        set_sc( SDL_SCANCODE_LALT );
    if ( kb->mkey._KbMkeyU._KbMkeyS.l_shift || kb->mkey._KbMkeyU._KbMkeyS.r_shift )
        set_sc( SDL_SCANCODE_LSHIFT );

    for ( i = 0; i < kb->nb_keycode && i < MAX_KEYCODES; i++ )
    {
        raw = ( u16 )( kb->keycode[i] & ~( KB_RAWDAT | KB_KEYPAD ) );
        if ( raw <= 3 )
            continue;
        set_sc( ( SDL_Scancode ) raw );
    }
}

void libkey_ps3_after_init( SDL_Window * window )
{
    ( void ) window;
    ioPadInit( 7 );
    memset( &ps3_last_pad, 0, sizeof( ps3_last_pad ) );
    ps3_have_pad = 0;
    ps3_kb_ok = 0;
    if ( ioKbInit( 1 ) == 0 )
    {
        ioKbSetCodeType( 0, KB_CODETYPE_RAW );
        ioKbSetReadMode( 0, KB_RMODE_PACKET );
        ps3_kb_ok = 1;
    }
    libkey_ps3_after_events();
}

void libkey_ps3_after_events( void )
{
    const bool * sdl;
    int n = 0, copy;
    padInfo info;
    padData pad;
    KbInfo kbinfo;
    KbData kb;

    sdl = SDL_GetKeyboardState( &n );
    copy = n;
    if ( copy > SDL_SCANCODE_COUNT )
        copy = SDL_SCANCODE_COUNT;
    memset( ps3_keystate, 0, sizeof( ps3_keystate ) );
    if ( sdl && copy > 0 )
        memcpy( ps3_keystate, sdl, ( size_t ) copy * sizeof( bool ) );

    ps3_keystate[ SDL_SCANCODE_ESCAPE ] = false;

    if ( ps3_kb_ok )
    {
        memset( &kbinfo, 0, sizeof( kbinfo ) );
        memset( &kb, 0, sizeof( kb ) );
        ioKbGetInfo( &kbinfo );
        if ( kbinfo.status[0] && ioKbRead( 0, &kb ) == 0 )
            apply_kb( &kb );
    }

    memset( &info, 0, sizeof( info ) );
    memset( &pad, 0, sizeof( pad ) );
    ioPadGetInfo( &info );
    if ( info.status[0] )
    {
        ioPadGetData( 0, &pad );
        /* ioPadGetData only fills when the state changes; keep the last held. */
        if ( pad.len )
        {
            ps3_last_pad = pad;
            ps3_have_pad = 1;
        }
        else if ( ps3_have_pad )
            pad = ps3_last_pad;
        apply_pad( &pad );
    }

    keystate = ps3_keystate;
}
