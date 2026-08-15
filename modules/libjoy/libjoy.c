/*
 *  Copyright © 2006-2013 SplinterGU (Fenix/Bennugd)
 *  Copyright © 2002-2006 Fenix Team (Fenix)
 *  Copyright © 1999-2002 José Luis Cebrián Pagüe (Fenix)
 *
 *  This file is part of Bennu - Game Development
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

/* --------------------------------------------------------------------------- */
/* Thanks Sandman for suggest on openjoys at initialization time               */
/* --------------------------------------------------------------------------- */
/* Credits SplinterGU/Sandman 2007-2009                                        */
/* --------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include "sdl3_compat.h"

/* --------------------------------------------------------------------------- */

#include "bgddl.h"

#include "bgdrtm.h"

#include "files.h"
#include "xstrings.h"

/* --------------------------------------------------------------------------- */

#include "libjoy_exports.h"

/* --------------------------------------------------------------------------- */

#ifdef TARGET_CAANOO
#include "caanoo/te9_tf9_hybrid_driver.c"

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x):(x))
#endif

#endif

/* --------------------------------------------------------------------------- */

#define MAX_JOYS    32

static int _max_joys = 0;
static SDL_Joystick * _joysticks[MAX_JOYS];
static SDL_JoystickID _joystick_ids[MAX_JOYS];
static char _joystick_names[MAX_JOYS][128];
static int _selected_joystick = -1;

static int libjoy_valid( int joy )
{
    return joy >= 0 && joy < _max_joys && _joysticks[ joy ];
}

static void libjoy_remember_name( int slot )
{
    const char * name = NULL;
    _joystick_names[ slot ][ 0 ] = 0;
    if ( slot < 0 || slot >= MAX_JOYS ) return;
    if ( _joysticks[ slot ] )
        name = SDL_GetJoystickName( _joysticks[ slot ] );
    if ( !name && _joystick_ids[ slot ] )
        name = SDL_GetJoystickNameForID( _joystick_ids[ slot ] );
    if ( name && name[ 0 ] )
        snprintf( _joystick_names[ slot ], sizeof( _joystick_names[ slot ] ), "%s", name );
#ifdef __EMSCRIPTEN__
    else
        snprintf( _joystick_names[ slot ], sizeof( _joystick_names[ slot ] ), "Gamepad" );
#endif
}

#ifdef __EMSCRIPTEN__
/* SoRR snapshots NUMBER_JOY() into joytemp during MENU. The browser Gamepad
 * API reports 0 pads until a button is pressed, so that snapshot stayed 0
 * and FALTA_JOY kept showing "GamePad not found" even after the pad worked. */
static void libjoy_placeholder( void )
{
    if ( _max_joys > 0 ) return;
    _max_joys = 1;
    _joysticks[ 0 ] = NULL;
    _joystick_ids[ 0 ] = 0;
    snprintf( _joystick_names[ 0 ], sizeof( _joystick_names[ 0 ] ), "Gamepad" );
}
#endif

/* --------------------------------------------------------------------------- */
/* libjoy_num ()                                                               */
/* Returns the number of joysticks present in the system                       */
/* --------------------------------------------------------------------------- */

int libjoy_num( void )
{
    return _max_joys ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_name (int JOY)                                                       */
/* Returns the name for a given joystick present in the system                 */
/* --------------------------------------------------------------------------- */

int libjoy_name( int joy )
{
    int result;
    if ( joy < 0 || joy >= _max_joys )
    {
        result = string_new( "" );
    }
    else
    {
        const char * name = NULL;
        if ( _joysticks[ joy ] )
            name = SDL_GetJoystickName( _joysticks[ joy ] );
        if ( !name && _joystick_ids[ joy ] )
            name = SDL_GetJoystickNameForID( _joystick_ids[ joy ] );
        if ( ( !name || !name[ 0 ] ) && _joystick_names[ joy ][ 0 ] )
            name = _joystick_names[ joy ];
#ifdef __EMSCRIPTEN__
        if ( !name || !name[ 0 ] )
            name = "Gamepad";
#endif
        result = string_new( name ? name : "" );
    }
    string_use( result );
    return result;
}

/* --------------------------------------------------------------------------- */
/* libjoy_select (int JOY)                                                     */
/* Returns the selected joystick number                                        */
/* --------------------------------------------------------------------------- */

int libjoy_select( int joy )
{
    return ( _selected_joystick = joy );
}

/* --------------------------------------------------------------------------- */
/* libjoy_buttons ()                                                           */
/* Returns the selected joystick total buttons                                 */
/* --------------------------------------------------------------------------- */

int libjoy_buttons( void )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
#ifdef TARGET_CAANOO
        if ( _selected_joystick == 0 ) return 21;
#endif
        return SDL_GetNumJoystickButtons( _joysticks[ _selected_joystick ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_axes ()                                                              */
/* Returns the selected joystick total axes                                    */
/* --------------------------------------------------------------------------- */

int libjoy_axes( void )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
        return SDL_GetNumJoystickAxes( _joysticks[ _selected_joystick ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_button ( int button )                                            */
/* Returns the selected joystick state for the given button                    */
/* --------------------------------------------------------------------------- */

int libjoy_get_button( int button )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
#ifdef TARGET_CAANOO
        if ( _selected_joystick == 0 )
        {
            int vax;

            switch ( button )
            {
                case    1: /* UPLF                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) < -16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) < -16384 );
                case    3: /* DWLF                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) >  16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) < -16384 );
                case    5: /* DWRT                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) >  16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) >  16384 );
                case    7: /* UPRT                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) < -16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) >  16384 );
                case    0: /* UP                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) < -16384 && ABS( vax ) < 16384 );
                case    4: /* DW                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) >  16384 && ABS( vax ) < 16384 );
                case    2: /* LF                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) < -16384 && ABS( vax ) < 16384 );
                case    6: /* RT                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) >  16384 && ABS( vax ) < 16384 );

                case    8:  /* MENU->HOME           */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 6 ) );
                case    9:  /* SELECT->HELP-II      */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 9 ) );
                case    10: /* L                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 4 ) );
                case    11: /* R                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 5 ) );
                case    12: /* A                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 0 ) );
                case    13: /* B                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 2 ) );
                case    14: /* X                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 1 ) );
                case    15: /* Y                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 3 ) );
                case    16: /* VOLUP                */  return ( 0 );
                case    17: /* VOLDOWN              */  return ( 0 );
                case    18: /* CLICK                */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 10 ) );
                case    19: /* POWER-LOCK  (CAANOO) */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 7 ) ); /* Only Caanoo */
                case    20: /* HELP-I      (CAANOO) */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 8 ) ); /* Only Caanoo */
                default:                                return ( 0 );
            }
        }
#endif
        return SDL_GetJoystickButton( _joysticks[ _selected_joystick ], button ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_position ( int axis )                                            */
/* Returns the selected joystick state for the given axis                      */
/* --------------------------------------------------------------------------- */

int libjoy_get_position( int axis )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
        return SDL_GetJoystickAxis( _joysticks[ _selected_joystick ], axis ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_hats ()                                                              */
/* Returns the total number of POV hats of the current selected joystick       */
/* --------------------------------------------------------------------------- */

int libjoy_hats( void )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
        return SDL_GetNumJoystickHats( _joysticks[ _selected_joystick ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_balls ()                                                             */
/* Returns the total number of balls of the current selected joystick          */
/* --------------------------------------------------------------------------- */

int libjoy_balls( void )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
        return SDL_GetNumJoystickBalls( _joysticks[ _selected_joystick ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_hat (int HAT)                                                    */
/* Returns the state of the specfied hat on the current selected joystick      */
/* --------------------------------------------------------------------------- */

int libjoy_get_hat( int hat )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
        if ( hat >= 0 && hat <= SDL_GetNumJoystickHats( _joysticks[ _selected_joystick ] ) )
        {
            return SDL_GetJoystickHat( _joysticks[ _selected_joystick ], hat ) ;
        }
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_ball (int BALL, int* dx, int* dy)                                */
/* Returns the state of the specfied ball on the current selected joystick     */
/* --------------------------------------------------------------------------- */

int libjoy_get_ball( int ball, int * dx, int * dy )
{
    if ( libjoy_valid( _selected_joystick ) )
    {
        if ( ball >= 0 && ball <= SDL_GetNumJoystickBalls( _joysticks[ _selected_joystick ] ) )
        {
            return SDL_GetJoystickBall( _joysticks[ _selected_joystick ], ball, dx, dy ) ? 0 : -1 ;
        }
    }
    return -1 ;
}

/* --------------------------------------------------------------------------- */

int libjoy_get_accel( int * x, int * y, int * z )
{
#ifdef TARGET_CAANOO
    if ( _selected_joystick == 0 )
    {
        KIONIX_ACCEL_read_LPF_g( x, y, z );
    }
    return 0;
#else
    return -1;
#endif
}

/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------- */
/* libjoy_buttons_specific (int JOY)                                           */
/* Returns the selected joystick total buttons                                 */
/* --------------------------------------------------------------------------- */

int libjoy_buttons_specific( int joy )
{
    if ( libjoy_valid( joy ) )
    {
#ifdef TARGET_CAANOO
        if ( joy == 0 ) return 21;
#endif
        return SDL_GetNumJoystickButtons( _joysticks[ joy ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_axes_specific (int JOY)                                              */
/* Returns the selected joystick total axes                                    */
/* --------------------------------------------------------------------------- */

int libjoy_axes_specific( int joy )
{
    if ( libjoy_valid( joy ) )
    {
        return SDL_GetNumJoystickAxes( _joysticks[ joy ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_button_specific (int JOY, int button)                            */
/* Returns the selected joystick state for the given button                    */
/* --------------------------------------------------------------------------- */

int libjoy_get_button_specific( int joy, int button )
{
    if ( libjoy_valid( joy ) )
    {
#ifdef TARGET_CAANOO
        if ( button >= 0 && ( ( joy == 0 && button <= 21 ) || ( joy != 0 && SDL_GetNumJoystickButtons( _joysticks[ joy ] ) ) ) )
#else
        if ( button >= 0 && button <= SDL_GetNumJoystickButtons( _joysticks[ joy ] ) )
#endif
        {
#ifdef TARGET_CAANOO
            if ( joy == 0 )
            {
                int vax;

                switch ( button )
                {
                    case    1: /* UPLF                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) < -16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) < -16384 );
                    case    3: /* DWLF                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) >  16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) < -16384 );
                    case    5: /* DWRT                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) >  16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) >  16384 );
                    case    7: /* UPRT                  */  return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) < -16384 && SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) >  16384 );
                    case    0: /* UP                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) < -16384 && ABS( vax ) < 16384 );
                    case    4: /* DW                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) >  16384 && ABS( vax ) < 16384 );
                    case    2: /* LF                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) < -16384 && ABS( vax ) < 16384 );
                    case    6: /* RT                    */  vax = SDL_GetJoystickAxis( _joysticks[ 0 ], 1 ) ; return ( SDL_GetJoystickAxis( _joysticks[ 0 ], 0 ) >  16384 && ABS( vax ) < 16384 );

                    case    8:  /* MENU->HOME           */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 6 ) );
                    case    9:  /* SELECT->HELP-II      */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 9 ) );
                    case    10: /* L                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 4 ) );
                    case    11: /* R                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 5 ) );
                    case    12: /* A                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 0 ) );
                    case    13: /* B                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 2 ) );
                    case    14: /* X                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 1 ) );
                    case    15: /* Y                    */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 3 ) );
                    case    16: /* VOLUP                */  return ( 0 );
                    case    17: /* VOLDOWN              */  return ( 0 );
                    case    18: /* CLICK                */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 10 ) );
                    case    19: /* POWER-LOCK  (CAANOO) */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 7 ) ); /* Only Caanoo */
                    case    20: /* HELP-I      (CAANOO) */  return ( SDL_GetJoystickButton( _joysticks[ 0 ], 8 ) ); /* Only Caanoo */
                    default:                                return ( 0 );
                }
            }
#endif
            return SDL_GetJoystickButton( _joysticks[ joy ], button ) ;
        }
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_position_specific (int JOY, int axis)                            */
/* Returns the selected joystick state for the given axis                      */
/* --------------------------------------------------------------------------- */

int libjoy_get_position_specific( int joy, int axis )
{
    if ( libjoy_valid( joy ) )
    {
        if ( axis >= 0 && axis <= SDL_GetNumJoystickAxes( _joysticks[ joy ] ) )
        {
            return SDL_GetJoystickAxis( _joysticks[ joy ], axis ) ;
        }
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* Added by Sandman */
/* --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------- */
/* libjoy_hats_specific (int JOY)                                              */
/* Returns the total number of POV hats of the specified joystick              */
/* --------------------------------------------------------------------------- */

int libjoy_hats_specific( int joy )
{
    if ( libjoy_valid( joy ) )
    {
        return SDL_GetNumJoystickHats( _joysticks[ joy ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_balls_specific (int JOY)                                             */
/* Returns the total number of balls of the specified joystick                 */
/* --------------------------------------------------------------------------- */

int libjoy_balls_specific( int joy )
{
    if ( libjoy_valid( joy ) )
    {
        return SDL_GetNumJoystickBalls( _joysticks[ joy ] ) ;
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_hat_specific (int JOY, int HAT)                                  */
/* Returns the state of the specfied hat on the specified joystick             */
/* --------------------------------------------------------------------------- */

int libjoy_get_hat_specific( int joy, int hat )
{
    if ( libjoy_valid( joy ) )
    {
        if ( hat >= 0 && hat <= SDL_GetNumJoystickHats( _joysticks[ joy ] ) )
        {
            return SDL_GetJoystickHat( _joysticks[ joy ], hat ) ;
        }
    }
    return 0 ;
}

/* --------------------------------------------------------------------------- */
/* libjoy_get_ball_specific (int JOY, int BALL, int* dx, int* dy)              */
/* Returns the state of the specfied ball on the specified joystick            */
/* --------------------------------------------------------------------------- */

int libjoy_get_ball_specific( int joy, int ball, int * dx, int * dy )
{
    if ( libjoy_valid( joy ) )
    {
        if ( ball >= 0 && ball <= SDL_GetNumJoystickBalls( _joysticks[ joy ] ) )
        {
            return SDL_GetJoystickBall( _joysticks[ joy ], ball, dx, dy ) ? 0 : -1 ;
        }
    }
    return -1 ;
}

/* --------------------------------------------------------------------------- */

int libjoy_get_accel_specific( int joy, int * x, int * y, int * z )
{
#ifdef TARGET_CAANOO
    if ( joy == 0 )
    {
        KIONIX_ACCEL_read_LPF_g( x, y, z );
	    return 0;
    }
#endif
    return -1;
}

/* --------------------------------------------------------------------------- */
/* Funciones de inicializacion del modulo/plugin                               */
/* --------------------------------------------------------------------------- */

void  __bgdexport( libjoy, module_initialize )()
{
    int i;
    int count = 0;
    SDL_JoystickID * ids;

    if ( !SDL_WasInit( SDL_INIT_JOYSTICK ) )
    {
        SDL_InitSubSystem( SDL_INIT_JOYSTICK );
        SDL_SetJoystickEventsEnabled( true );
    }

    /* Open all joysticks */
    ids = SDL_GetJoysticks( &count );
    if ( ids )
    {
        _max_joys = count;
        if ( _max_joys > MAX_JOYS )
        {
            printf( "[JOY] Warning: maximum number of joysticks exceeded (%i>%i)", _max_joys, MAX_JOYS );
            _max_joys = MAX_JOYS;
        }

        for ( i = 0; i < _max_joys; i++ )
        {
            _joystick_ids[i] = ids[i];
            _joysticks[i] = SDL_OpenJoystick( ids[i] );
            if ( !_joysticks[ i ] ) printf( "[JOY] Failed to open joystick '%i'", i );
            libjoy_remember_name( i );
        }

        SDL_free( ids );
        SDL_UpdateJoysticks() ;
    }
    else
    {
        _max_joys = 0;
#ifndef __EMSCRIPTEN__
        return;
#endif
    }

#ifdef __EMSCRIPTEN__
    if ( _max_joys == 0 )
        libjoy_placeholder();
#endif

#ifdef TARGET_CAANOO
    KIONIX_ACCEL_init();

    if ( KIONIX_ACCEL_get_device_type() != DEVICE_TYPE_KIONIX_KXTF9 ) KIONIX_ACCEL_deinit();

    KXTF9_set_G_range(2);
    KXTF9_set_resolution(12);
    KXTF9_set_lpf_odr(400);

    KIONIX_ACCEL_enable_outputs();
#endif
}

#ifdef __EMSCRIPTEN__
/* Browser Gamepad API only exposes a pad after a button press, and the same
 * pad can reconnect with a new SDL id. Desktop still opens pads once at init. */
static int libjoy_id_in_list( SDL_JoystickID id, SDL_JoystickID * ids, int count )
{
    int i;
    for ( i = 0; i < count; i++ )
        if ( ids[ i ] == id ) return 1;
    return 0;
}

static int libjoy_name_taken( const char * name )
{
    int j;
    if ( !name || !name[ 0 ] ) return 0;
    for ( j = 0; j < _max_joys; j++ )
        if ( _joystick_names[ j ][ 0 ] && strcmp( _joystick_names[ j ], name ) == 0 )
            return 1;
    return 0;
}

/* Only recycle a slot when its SDL id really disappeared — not when
 * SDL_JoystickConnected() flickers for a frame (common in the browser). */
static int libjoy_first_dead_slot( SDL_JoystickID * ids, int count )
{
    int j;
    for ( j = 0; j < _max_joys; j++ )
    {
        if ( !_joysticks[ j ] ) return j;
        if ( !libjoy_id_in_list( _joystick_ids[ j ], ids, count ) ) return j;
    }
    return -1;
}

static void libjoy_refresh( void )
{
    int count = 0, i;
    SDL_JoystickID * ids;

    SDL_UpdateJoysticks();
    ids = SDL_GetJoysticks( &count );
    if ( !ids )
        return;

    for ( i = 0; i < count; i++ )
    {
        int slot, dead;
        SDL_Joystick * js;
        const char * name;

        if ( libjoy_id_in_list( ids[ i ], _joystick_ids, _max_joys ) )
            continue;

        js = SDL_OpenJoystick( ids[ i ] );
        if ( !js || SDL_GetNumJoystickButtons( js ) <= 0 )
        {
            if ( js ) SDL_CloseJoystick( js );
            continue;
        }

        name = SDL_GetJoystickName( js );
        dead = libjoy_first_dead_slot( ids, count );
        if ( dead < 0 && libjoy_name_taken( name ) )
        {
            /* Same physical pad under a new id while the old slot is still listed. */
            SDL_CloseJoystick( js );
            continue;
        }

        slot = ( dead >= 0 ) ? dead : _max_joys;
        if ( slot >= MAX_JOYS )
        {
            SDL_CloseJoystick( js );
            break;
        }

        if ( _joysticks[ slot ] ) SDL_CloseJoystick( _joysticks[ slot ] );
        _joystick_ids[ slot ] = ids[ i ];
        _joysticks[ slot ] = js;
        libjoy_remember_name( slot );
        if ( slot == _max_joys )
            _max_joys++;
    }

    SDL_free( ids );
}
#endif

/* ----------------------------------------------------------------- */

void  __bgdexport( libjoy, module_finalize )()
{
    int i;

#ifdef TARGET_CAANOO
    KIONIX_ACCEL_deinit();
#endif

    for ( i = 0; i < _max_joys; i++ )
        if ( _joysticks[ i ] ) SDL_CloseJoystick( _joysticks[ i ] ) ;

    if ( SDL_WasInit( SDL_INIT_JOYSTICK ) ) SDL_QuitSubSystem( SDL_INIT_JOYSTICK );

}

HOOK __bgdexport( libjoy, handler_hooks )[] =
{
#ifdef __EMSCRIPTEN__
    { 4900, libjoy_refresh },
#endif
    {    0, NULL           }
};

/* ----------------------------------------------------------------- */
