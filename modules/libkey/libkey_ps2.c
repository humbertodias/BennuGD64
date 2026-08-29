#include <stdint.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <libpad.h>

#include "libkey_ps2.h"

void gr_stats_toggle( void ) __attribute__((weak));

extern const bool * keystate;

#define PAD_DIGITAL_IDLE  0x00
#define PAD_THRESH_LO     64
#define PAD_THRESH_HI     192

static bool ps2_keystate[ SDL_SCANCODE_COUNT ];
static char pad_buf[ 256 ] __attribute__( ( aligned( 64 ) ) );
static int pad_ready;
static int pad_wait;
static int last_paddata;

static void apply_pad( uint32_t paddata )
{
    memset( ps2_keystate, 0, sizeof( ps2_keystate ) );

    if ( paddata & PAD_UP )    ps2_keystate[ SDL_SCANCODE_UP ]    = 1;
    if ( paddata & PAD_DOWN )  ps2_keystate[ SDL_SCANCODE_DOWN ]  = 1;
    if ( paddata & PAD_LEFT )  ps2_keystate[ SDL_SCANCODE_LEFT ]  = 1;
    if ( paddata & PAD_RIGHT ) ps2_keystate[ SDL_SCANCODE_RIGHT ] = 1;

    if ( paddata & ( PAD_CROSS | PAD_R1 ) )
    {
        ps2_keystate[ SDL_SCANCODE_C ]     = 1;
        ps2_keystate[ SDL_SCANCODE_LCTRL ] = 1;
    }
    if ( paddata & ( PAD_CIRCLE | PAD_R2 ) )
    {
        ps2_keystate[ SDL_SCANCODE_V ]    = 1;
        ps2_keystate[ SDL_SCANCODE_LALT ] = 1;
    }
    if ( paddata & ( PAD_SQUARE | PAD_L1 ) )
    {
        ps2_keystate[ SDL_SCANCODE_X ]     = 1;
        ps2_keystate[ SDL_SCANCODE_SPACE ] = 1;
    }
    if ( paddata & ( PAD_TRIANGLE | PAD_L2 ) )
        ps2_keystate[ SDL_SCANCODE_B ] = 1;

    if ( paddata & PAD_START )  ps2_keystate[ SDL_SCANCODE_RETURN ] = 1;
    if ( paddata & PAD_SELECT ) ps2_keystate[ SDL_SCANCODE_ESCAPE ] = 1;
}

void libkey_ps2_after_events( void )
{
    struct padButtonStatus btn;
    int state;
    uint32_t paddata;
    uint8_t lx, ly;

    keystate = ps2_keystate;

    if ( !pad_ready )
    {
        if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
            return;

        if ( pad_wait < 8 )
        {
            pad_wait++;
            return;
        }

        SifLoadModule( "rom0:SIO2MAN", 0, NULL );
        SifLoadModule( "rom0:PADMAN", 0, NULL );
        padInit( 0 );
        if ( padPortOpen( 0, 0, pad_buf ) )
            pad_ready = 1;
        return;
    }

    state = padGetState( 0, 0 );
    if ( state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1 )
        return;

    if ( padRead( 0, 0, &btn ) == 0 )
        return;

    paddata = 0xffff ^ btn.btns;
    lx = btn.ljoy_h;
    ly = btn.ljoy_v;

    if ( !( lx == PAD_DIGITAL_IDLE && ly == PAD_DIGITAL_IDLE ) )
    {
        if ( ly < PAD_THRESH_LO ) paddata |= PAD_UP;
        if ( ly > PAD_THRESH_HI ) paddata |= PAD_DOWN;
        if ( lx < PAD_THRESH_LO ) paddata |= PAD_LEFT;
        if ( lx > PAD_THRESH_HI ) paddata |= PAD_RIGHT;
    }

    /* Select+L1+R1: FPS/mem overlay (F12 on keyboard does the same).
     * Strip the combo so Select does not become Escape in the game. */
    if ( ( paddata & ( PAD_SELECT | PAD_L1 | PAD_R1 ) ) == ( PAD_SELECT | PAD_L1 | PAD_R1 ) )
    {
        if ( gr_stats_toggle &&
             ( last_paddata & ( PAD_SELECT | PAD_L1 | PAD_R1 ) ) != ( PAD_SELECT | PAD_L1 | PAD_R1 ) )
            gr_stats_toggle();
    }

    last_paddata = (int) paddata;
    if ( ( paddata & ( PAD_SELECT | PAD_L1 | PAD_R1 ) ) == ( PAD_SELECT | PAD_L1 | PAD_R1 ) )
        paddata &= ~( PAD_SELECT | PAD_L1 | PAD_R1 );
    apply_pad( paddata );
}

int libkey_ps2_scan_code( int *ascii )
{
    if ( ascii )
        *ascii = 0;

    if ( last_paddata & PAD_START )
    {
        if ( ascii ) *ascii = 13;
        return 28;
    }
    if ( last_paddata & PAD_UP )    return 72;
    if ( last_paddata & PAD_DOWN )  return 80;
    if ( last_paddata & PAD_LEFT )  return 75;
    if ( last_paddata & PAD_RIGHT ) return 77;
    if ( last_paddata & ( PAD_CROSS | PAD_R1 ) )
    {
        if ( ascii ) *ascii = 'c';
        return 46;
    }
    if ( last_paddata & ( PAD_CIRCLE | PAD_R2 ) )
    {
        if ( ascii ) *ascii = 'v';
        return 47;
    }
    if ( last_paddata & ( PAD_SQUARE | PAD_L1 ) )
    {
        if ( ascii ) *ascii = 'x';
        return 45;
    }
    if ( last_paddata & ( PAD_TRIANGLE | PAD_L2 ) )
    {
        if ( ascii ) *ascii = 'b';
        return 48;
    }
    return 0;
}
