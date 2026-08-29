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

/*
 * INCLUDES
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#else
#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
#endif

#include "bgdi.h"
#include "bgdrtm.h"
#include "xstrings.h"
#include "dirs.h"
#include "bgd_platform.h"

#ifdef TARGET_ANDROID
#include "main_android.h"
#endif
#ifdef TARGET_SWITCH
#include "main_switch.h"
#endif
#ifdef TARGET_DC
#include "main_dc.h"
#endif
#ifdef TARGET_PSP
#include "main_psp.h"
#endif
#ifdef TARGET_PS2
#include "main_ps2.h"
#endif
#ifdef TARGET_PANDORA
#include "main_pandora.h"
#endif
#ifdef TARGET_WII
#include "main_wii.h"
#endif
#ifdef TARGET_WIN32
#include "main_win32.h"
#endif

/* ---------------------------------------------------------------------- */

static char * dcb_exts[] = { ".dcb", ".dat", ".bin", NULL };

static int standalone  = 0;  /* 1 only if this is an standalone interpreter   */
static int embedded    = 0;  /* 1 only if this is a stub with an embedded DCB */

/* ---------------------------------------------------------------------- */

/*
 *  FUNCTION : main
 *
 *  Main function
 *
 *  PARAMS:
 *      INT n: ERROR LEVEL to return to OS
 *
 *  RETURN VALUE:
 *      No value
 *
 */

int main( int argc, char *argv[] )
{
    char * filename = NULL, dcbname[ __MAX_PATH ], *ptr, *arg0, *ext ;
    int i, j, ret = -1;
    file * fp = NULL;
    INSTANCE * mainproc_running;
    dcb_signature dcb_signature;

    /* disable stdout buffering */
#ifndef TARGET_PS2
    setvbuf( stdout, NULL, _IONBF, BUFSIZ );
#endif

#ifdef TARGET_ANDROID
    {
        char * android_dcb = bgdi_android_startup( argc, argv, &standalone );
        if ( android_dcb )
            filename = android_dcb;
    }
#endif

#ifdef TARGET_SWITCH
    {
        char * switch_dcb = bgdi_switch_startup( argc, argv, &standalone );
        if ( switch_dcb )
            filename = switch_dcb;
    }
#endif

#ifdef TARGET_DC
    {
        char * dc_dcb = bgdi_dc_startup( argc, argv, &standalone );
        if ( dc_dcb )
            filename = dc_dcb;
    }
#endif

#ifdef TARGET_PSP
    {
        char * psp_dcb = bgdi_psp_startup( argc, argv, &standalone );
        if ( psp_dcb )
            filename = psp_dcb;
    }
#endif

#ifdef TARGET_PS2
    {
        static char * ps2_argv[2] = { "bgdi", NULL };
        char * ps2_dcb;

        if ( argc < 1 || !argv || !argv[0] )
        {
            argc = 1;
            argv = ps2_argv;
        }

        ps2_dcb = bgdi_ps2_startup( argc, argv, &standalone );
        if ( ps2_dcb )
            filename = ps2_dcb;
        if ( filename && argv && argv[0] )
            bgdi_ps2_use_device_argv0( filename, argv );
    }
#endif

#ifdef TARGET_PANDORA
    {
        char * pandora_dcb = bgdi_pandora_startup( argc, argv, &standalone );
        if ( pandora_dcb )
            filename = pandora_dcb;
    }
#endif

#ifdef TARGET_WII
    {
        /* Dolphin File→Open and HBC pass argc=0 / argv=NULL. main() later does
         * argv[0] = filename, which is a write to 0x00000000. */
        static char * wii_argv[2] = { "bgdi", NULL };
        char * wii_dcb;

        if ( argc < 1 || !argv || !argv[0] )
        {
            argc = 1;
            argv = wii_argv;
        }

        wii_dcb = bgdi_wii_startup( argc, argv, &standalone );
        if ( wii_dcb )
            filename = wii_dcb;
    }
#endif

    /* get my executable name */

#ifdef TARGET_WIN32
    arg0 = bgdi_win32_resolve_argv0( argc, argv );
#else
    if ( argc < 1 || !argv || !argv[0] )
        arg0 = strdup( "bgdi" );
    else
        arg0 = strdup( argv[0] );
#endif

    ptr = arg0 + strlen( arg0 );
    while ( ptr > arg0 && ptr[-1] != '\\' && ptr[-1] != '/' ) ptr-- ;

    appexename = strdup( ptr );

    /* get executable full pathname  */
    fp = NULL;
    appexefullpath = getfullpath( arg0 );
    if ( ( !strchr( arg0, '\\' ) && !strchr( arg0, '/' ) ) )
    {
        struct stat st;
        if ( stat( appexefullpath, &st ) || !S_ISREG( st.st_mode ) )
        {
            char *p = whereis( appexename );
            if ( p )
            {
                char * tmp = calloc( 1, strlen( p ) + strlen( appexename ) + 2 );
                free( appexefullpath );
                sprintf( tmp, "%s/%s", p, appexename );
                appexefullpath = getfullpath( tmp );
                free( tmp );
            }
        }
    }

    /* get pathname of executable */
    if ( !appexefullpath ) appexefullpath = strdup( "" );
    ptr = ( appexename && appexefullpath ) ? strstr( appexefullpath, appexename ) : NULL;
    if ( !ptr ) ptr = appexefullpath + strlen( appexefullpath );
    appexepath = calloc( 1, (size_t)( ptr - appexefullpath ) + 1 );
    strncpy( appexepath, appexefullpath, ptr - appexefullpath );

#ifdef BGD_STANDALONE_INTERPRETER
    standalone = 1 ;
#else
    standalone = ( strncmpi( appexename, "bgdi", 4 ) == 0 ) ;
#endif

    /* add binary path */
    file_addp( appexepath );

    if ( !standalone )
    {
        /* Hand-made interpreter: search for DCB at EOF */
        fp = file_open( appexefullpath, "rb0" );
        if ( fp )
        {
            file_seek( fp, -( int )sizeof( dcb_signature ), SEEK_END );
            file_read( fp, &dcb_signature, sizeof( dcb_signature ) );

            if ( strcmp( dcb_signature.magic, DCB_STUB_MAGIC ) == 0 )
            {
                ARRANGE_DWORD( &dcb_signature.dcb_offset );
                embedded = 1;
            }
        }

        filename = appexefullpath;
    }

    if ( standalone )
    {
        /* Calling BGDI.EXE so we must get all command line params */

        for ( i = 1 ; i < argc ; i++ )
        {
            if ( argv[i][0] == '-' )
            {
                j = 1 ;
                while ( argv[i][j] )
                {
                    if ( argv[i][j] == 'd' ) debug++;
                    if ( argv[i][j] == 'i' )
                    {
                        if ( argv[i][j+1] == 0 )
                        {
                            if ( i == argc - 1 )
                            {
                                fprintf( stderr, "You must provide a directory" ) ;
                                exit( 0 );
                            }
                            file_addp( argv[i+1] );
                            i++ ;
                            break ;
                        }
                        file_addp( &argv[i][j + 1] ) ;
                        break ;
                    }
                    j++ ;
                }
            }
            else
            {
                if ( !filename )
                {
                    filename = argv[i] ;
                    if ( i < argc - 1 ) memmove( &argv[i], &argv[i+1], sizeof( char* ) * ( argc - i - 1 ) ) ;
                    argc-- ;
                    i-- ;
                }
            }
        }

        if ( !filename )
        {
            printf( BGDI_VERSION "\n"
                    "Bennu Game Development Interpreter\n"
                    "\n"
                    "Copyright (c) 2006-2013 SplinterGU (Fenix/BennuGD)\n"
                    "Copyright (c) 2002-2006 Fenix Team (Fenix)\n"
                    "Copyright (c) 1999-2002 José Luis Cebrián Pagüe (Fenix)\n"
                    "\n"
                    "Usage: %s [options] <data code block file>[.dcb]\n"
                    "\n"
                    "   -d       Activate DEBUG mode (several -d for increment debug level)\n"
                    "   -i dir   Adds the directory to the PATH\n"
                    "\n"
                    "This software is provided 'as-is', without any express or implied\n"
                    "warranty. In no event will the authors be held liable for any damages\n"
                    "arising from the use of this software.\n"
                    "\n"
                    "Permission is granted to anyone to use this software for any purpose,\n"
                    "including commercial applications, and to alter it and redistribute it\n"
                    "freely, subject to the following restrictions:\n"
                    "\n"
                    "   1. The origin of this software must not be misrepresented; you must not\n"
                    "   claim that you wrote the original software. If you use this software\n"
                    "   in a product, an acknowledgment in the product documentation would be\n"
                    "   appreciated but is not required.\n"
                    "\n"
                    "   2. Altered source versions must be plainly marked as such, and must not be\n"
                    "   misrepresented as being the original software.\n"
                    "\n"
                    "   3. This notice may not be removed or altered from any source\n"
                    "   distribution.\n"
                    , appexename ) ;
            return -1 ;
        }
    }

    /* Initialization (modules needed before dcb_load) */

    string_init() ;
    init_c_type() ;

    /* Init application title for windowed modes */

    strcpy( dcbname, filename ) ;

    ptr = filename + strlen( filename );
    while ( ptr > filename && ptr[-1] != '\\' && ptr[-1] != '/' ) ptr-- ;

    appname = strdup( ptr ) ;
    if ( strlen( appname ) > 3 )
    {
        char ** dcbext = dcb_exts, *ext = &appname[ strlen( appname ) - 4 ];
#ifdef TARGET_WIN32
        if ( !bgdi_win32_strip_exe_suffix( ext ) )
#endif
        while ( dcbext && *dcbext )
        {
            if ( !strncmpi( ext, *dcbext, 4 ) )
            {
                *ext = '\0';
                break;
            }
            dcbext++;
        }
    }

#ifdef __DEBUG__
printf( "appname        %s\n", appname);
printf( "appexename     %s\n", appexename);
printf( "appexepath     %s\n", appexepath);
printf( "appexefullpath %s\n", appexefullpath);
printf( "dcbname        %s\n", dcbname);
fflush(stdout);
#endif

    if ( !embedded )
    {
        /* First try to load directly (we expect myfile.dcb) */
        if ( !dcb_load( dcbname ) )
        {
            char ** dcbext = dcb_exts;
            int dcbloaded = 0;

            while ( dcbext && *dcbext )
            {
                strcpy( dcbname, appname ) ;
                strcat( dcbname, *dcbext ) ;
                if (( dcbloaded = dcb_load( dcbname ) ) ) break;
                dcbext++;
            }

            if ( !dcbloaded )
            {
                printf( "%s: doesn't exist or isn't version %d DCB compatible\n", filename, DCB_VERSION >> 8 ) ;
                return -1 ;
            }
        }
    }
    else
    {
        dcb_load_from( fp, dcbname, dcb_signature.dcb_offset );
    }

    /* If the dcb is not in debug mode */

    if ( dcb.data.NSourceFiles == 0 ) debug = 0;

    /* Initialization (modules needed after dcb_load) */

    sysproc_init() ;

#ifdef TARGET_WIN32
    bgdi_win32_hide_own_console();
#endif

    if ( argv && argc > 0 )
        argv[0] = filename;
    bgdrtm_entry( argc, argv );

    if ( mainproc )
    {
        mainproc_running = instance_new( mainproc, NULL ) ;
        ret = instance_go_all() ;
    }

    bgdrtm_exit( ret );

    free( appexename        );
    free( appexepath        );
    free( appexefullpath    );
    free( appname           );

    return ret;
}

