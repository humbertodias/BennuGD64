/*
 * Wii Remote extras that do not fit JOY_*: IR, battery, rumble, Balance Board.
 * Matching bennugd-wii modules/mod_wpad.
 */

#include "bgddl.h"

#ifdef TARGET_WII
#include <wiiuse/wiiuse.h>
#include <wiiuse/wpad.h>
#else
#include <SDL3/SDL.h>
#endif

static int modwpad_is_ready( INSTANCE * my, intptr_t * params );
static int modwpad_info( INSTANCE * my, intptr_t * params );
static int modwpad_info_bb( INSTANCE * my, intptr_t * params );
static int modwpad_rumble( INSTANCE * my, intptr_t * params );

#include "mod_wpad_exports.h"

#ifdef TARGET_WII
static int is_bb( int chan )
{
    u32 type;

    WPAD_Probe( chan, &type );
    return type == WPAD_EXP_WIIBOARD;
}
#endif

static int modwpad_is_ready( INSTANCE * my, intptr_t * params )
{
    ( void ) my;
#ifdef TARGET_WII
    {
        u32 type;
        int res = WPAD_Probe( ( int ) params[0], &type );
        if ( res == 0 )
            return 1;
        return res;
    }
#else
    /* Desktop stand-in: channel 0 is always present (mouse as IR). */
    return ( ( int ) params[0] == 0 ) ? 1 : -1;
#endif
}

static int modwpad_info( INSTANCE * my, intptr_t * params )
{
    ( void ) my;
#ifdef TARGET_WII
    {
        u32 type;
        WPADData * wd;
        int chan = ( int ) params[0];

        if ( WPAD_Probe( chan, &type ) != 0 )
            return 0;

        wd = WPAD_Data( chan );
        if ( !wd )
            return 0;

        switch ( ( int ) params[1] )
        {
            case WPAD_BATT:    return ( int ) WPAD_BatteryLevel( chan );
            case WPAD_X:       return ( int ) wd->ir.x;
            case WPAD_Y:       return ( int ) wd->ir.y;
            case WPAD_Z:       return ( int ) wd->ir.z;
            case WPAD_ANGLE:   return -( int )( wd->ir.angle * 1000.0 );
            case WPAD_PITCH:   return ( int )( wd->orient.pitch * 1000.0 );
            case WPAD_ROLL:    return ( int )( wd->orient.roll * 1000.0 );
            case WPAD_ACCELX:  return wd->accel.x;
            case WPAD_ACCELY:  return wd->accel.y;
            case WPAD_ACCELZ:  return wd->accel.z;
            case WPAD_IS_BB:   return is_bb( chan );
            default:           return 0;
        }
    }
#else
    {
        float mx = 0.0f, my = 0.0f;

        if ( ( int ) params[0] != 0 )
            return 0;

        SDL_GetMouseState( &mx, &my );
        switch ( ( int ) params[1] )
        {
            case WPAD_BATT:   return 200;
            case WPAD_X:      return ( int ) mx;
            case WPAD_Y:      return ( int ) my;
            case WPAD_Z:      return 1;
            case WPAD_ANGLE:  return 0;
            case WPAD_PITCH:  return 0;
            case WPAD_ROLL:   return 0;
            case WPAD_ACCELX: return 0;
            case WPAD_ACCELY: return 0;
            case WPAD_ACCELZ: return 0;
            case WPAD_IS_BB:  return 0;
            default:          return 0;
        }
    }
#endif
}

static int modwpad_info_bb( INSTANCE * my, intptr_t * params )
{
    ( void ) my;
#ifdef TARGET_WII
    {
        expansion_t exp;
        u32 type;
        int chan = ( int ) params[0];

        if ( !is_bb( chan ) )
            return 0;
        if ( WPAD_Probe( chan, &type ) != 0 )
            return 0;

        WPAD_Expansion( chan, &exp );
        switch ( ( int ) params[1] )
        {
            case WPAD_BATT: return ( int ) WPAD_BatteryLevel( chan );
            case WPAD_X:    return ( int ) exp.wb.x;
            case WPAD_Y:    return ( int ) exp.wb.y;
            case WPAD_WTL:  return ( int ) exp.wb.tl;
            case WPAD_WTR:  return ( int ) exp.wb.tr;
            case WPAD_WBL:  return ( int ) exp.wb.bl;
            case WPAD_WBR:  return ( int ) exp.wb.br;
            default:        return 0;
        }
    }
#else
    ( void ) params;
    return 0;
#endif
}

static int modwpad_rumble( INSTANCE * my, intptr_t * params )
{
    ( void ) my;
#ifdef TARGET_WII
    WPAD_Rumble( ( int ) params[0], ( int ) params[1] );
#else
    ( void ) params;
#endif
    return 0;
}
