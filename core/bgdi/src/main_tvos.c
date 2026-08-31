/*
 * Apple tvOS interpreter entry: bundle path, search path, default DCB.
 *
 * Assets live at the .app root (flat bundle). SDL_GetBasePath() is that
 * directory. Saves go to the app's Documents folder via SDL_GetPrefPath.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main_tvos.h"
#include "files.h"

static int tvos_readable( const char * path )
{
    FILE * test;

    if ( !path || !path[0] )
        return 0;
    test = fopen( path, "rb" );
    if ( !test )
        return 0;
    fclose( test );
    return 1;
}

char * bgdi_tvos_startup( int argc, char * argv[], int * standalone )
{
    static char dcb[ __MAX_PATH ];
    const char * base;
    const char * pref;

    ( void ) argv;

    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "metal" );

    base = SDL_GetBasePath();
    pref = SDL_GetPrefPath( "bennugd64", "bgdi" );

    if ( pref && pref[0] )
    {
        chdir( pref );
        file_addp( pref );
    }
    if ( base && base[0] )
        file_addp( base );
    file_addp( "." );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    if ( base && base[0] )
    {
        snprintf( dcb, sizeof( dcb ), "%smain.dcb", base );
        if ( tvos_readable( dcb ) )
        {
            fprintf( stderr, "bgdi: using %s\n", dcb );
            return dcb;
        }
        fprintf( stderr, "bgdi: missing %s\n", dcb );
    }

    if ( tvos_readable( "main.dcb" ) )
        return "main.dcb";

    fprintf( stderr, "bgdi: main.dcb not found in the app bundle\n" );
    return NULL;
}
