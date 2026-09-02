/*
 * Apple tvOS interpreter entry: bundle path, search path, default DCB.
 *
 * Assets live at the .app root (flat bundle). Relative fopen is resolved
 * against the DCB directory in files_tvos.c; absolute paths are unchanged.
 * Saves go to Documents via SDL_GetPrefPath.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main_tvos.h"
#include "files.h"
#include "files_tvos.h"

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

static void tvos_add_root( const char * path )
{
    if ( path && path[0] )
        file_addp( path );
}

static void tvos_use_dcb( const char * dcb_path, const char * bundle, const char * docs )
{
    const char * root;

    file_tvos_bind_root( dcb_path );
    root = file_tvos_root();
    if ( docs && docs[0] )
        chdir( docs );
    else if ( root && root[0] )
        chdir( root );

    tvos_add_root( root );
    if ( docs && docs[0] && ( !root || strcmp( docs, root ) != 0 ) )
        tvos_add_root( docs );
    if ( bundle && bundle[0] && ( !root || strcmp( bundle, root ) != 0 ) )
        tvos_add_root( bundle );
    file_addp( "." );
    fprintf( stderr, "bgdi: data root %s\n", root ? root : "(none)" );
}

char * bgdi_tvos_startup( int argc, char * argv[], int * standalone )
{
    static char dcb[ __MAX_PATH ];
    const char * base;
    const char * pref;

    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "metal" );

    base = SDL_GetBasePath();
    pref = SDL_GetPrefPath( "bennugd64", "bgdi" );
    file_tvos_set_roots( base, pref );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && argv[1][0] )
    {
        tvos_use_dcb( argv[1], base, pref );
        return NULL;
    }

    if ( pref && pref[0] )
    {
        snprintf( dcb, sizeof( dcb ), "%smain.dcb", pref );
        if ( tvos_readable( dcb ) )
        {
            fprintf( stderr, "bgdi: using %s\n", dcb );
            tvos_use_dcb( dcb, base, pref );
            return dcb;
        }
    }

    if ( base && base[0] )
    {
        snprintf( dcb, sizeof( dcb ), "%smain.dcb", base );
        if ( tvos_readable( dcb ) )
        {
            fprintf( stderr, "bgdi: using %s\n", dcb );
            tvos_use_dcb( dcb, base, pref );
            return dcb;
        }
        fprintf( stderr, "bgdi: missing %s\n", dcb );
    }

    if ( tvos_readable( "main.dcb" ) )
    {
        tvos_use_dcb( "main.dcb", base, pref );
        return "main.dcb";
    }

    fprintf( stderr, "bgdi: main.dcb not found in the app bundle or Documents\n" );
    tvos_use_dcb( NULL, base, pref );
    return NULL;
}
