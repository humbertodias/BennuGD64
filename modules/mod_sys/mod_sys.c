/*
 *  Copyright © 2006-2013 SplinterGU (Fenix/Bennugd)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bgddl.h"
#include "files.h"
#include "xstrings.h"

#ifndef WIN32
#include <unistd.h>
#if !defined(__EMSCRIPTEN__) && !defined(__SWITCH__) && !defined(_arch_dreamcast) && !defined(__PSP__)
#include <sys/wait.h>
#endif
#else
#include <process.h>
#endif

/* ---------------------------------------------------------------------- */

#include "mod_sys.h"

/* ---------------------------------------------------------------------- */

static int modsys_exec( INSTANCE * my, intptr_t * params )
{
    int mode = params[0];
    char * filename = ( char * ) string_get( params[1] );
    int argc = params[2];
    char ** argv;
    int n = 0;
#if !defined(WIN32) && !defined(__EMSCRIPTEN__) && !defined(__SWITCH__) && !defined(_arch_dreamcast) && !defined(__PSP__)
    pid_t child;
#endif
    int status = -1;

    // fill argv
    argv = ( char ** ) calloc( argc + 2, sizeof( char * ) );
    argv[0] = filename;
    for ( n = 0; n < argc; n++ )
        argv[n + 1] = ( char * ) string_get((( int * )( params[3] ) )[n] );

    // Execute program
#ifdef __EMSCRIPTEN__
    ( void ) mode;
    ( void ) argc;
    status = -1;
#elif defined(__SWITCH__)
    ( void ) mode;
    ( void ) argc;
    status = -1;
#elif defined(_arch_dreamcast)
    ( void ) mode;
    ( void ) argc;
    status = -1;
#elif defined(__PSP__)
    ( void ) mode;
    ( void ) argc;
    status = -1;
#elif defined(WIN32)
    status = spawnvp( mode, filename, ( const char ** )argv );
#else
    if (( child = fork() ) == -1 )
    {
        //Error
        status = -1 ;
    }
    else if ( child == 0 )
    {
        execvp( filename, ( const char ** )argv );
        exit(-1);
    }
    else
    {
        /* father */
        switch ( mode )
        {
            case _P_WAIT:
                if ( waitpid( child, &status, WUNTRACED ) != child )
                    status = -1;
                else
                    status = (int)(char)WEXITSTATUS(status);
                break;

            case _P_NOWAIT:
                status = child;
                break;
        }
    }
#endif

    // Free resources
    string_discard( params[1] );
    if ( argv ) free( argv );

    return ( status ) ;
}

/* ---------------------------------------------------------------------- */

static int modsys_getenv( INSTANCE * my, intptr_t * params )
{
    char *e ;
    int str ;

    if (( e = getenv( string_get( params[0] ) ) ) )
    {
        str = string_new( e ) ;
    }
    else
    {
        str = string_new( "" ) ;
    }

    string_discard( params[0] ) ;
    string_use( str ) ;
    return str ;
}

/* ---------------------------------------------------------------------- */

static const char * modsys_running_os_name( void )
{
#if defined(TARGET_EMSCRIPTEN)
    return "Web";
#elif defined(TARGET_DC)
    return "Dreamcast";
#elif defined(TARGET_PSP)
    return "PSP";
#elif defined(TARGET_PANDORA)
    return "Pandora";
#elif defined(TARGET_SWITCH)
    return "Switch";
#elif defined(TARGET_ANDROID)
    return "Android";
#elif defined(TARGET_WASI)
    return "WASI";
#elif defined(TARGET_IOS)
    return "iOS";
#elif defined(TARGET_WII)
    return "Wii";
#elif defined(TARGET_MAC)
    return "macOS";
#elif defined(TARGET_LINUX)
    return "Linux";
#elif defined(TARGET_BSD)
    return "BSD";
#elif defined(TARGET_BEOS)
    return "BeOS";
#elif defined(TARGET_WIN32) || defined(_WIN32)
    return "Windows";
#elif defined(TARGET_GP32)
    return "GP32";
#elif defined(TARGET_GP2X_WIZ)
    return "GP2X Wiz";
#elif defined(TARGET_GP2X)
    return "GP2X";
#elif defined(TARGET_CAANOO)
    return "Caanoo";
#elif defined(TARGET_DINGUX_A320)
    return "Dingux";
#else
    return "Unknown";
#endif
}

static int modsys_os_name( INSTANCE * my, intptr_t * params )
{
    int str;

    ( void ) my;
    ( void ) params;

    str = string_new( modsys_running_os_name() );
    string_use( str );
    return str;
}

/* ----------------------------------------------------------------- */
/* exports                                                           */
/* ----------------------------------------------------------------- */

#include "mod_sys_exports.h"

/* ----------------------------------------------------------------- */
