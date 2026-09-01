/*
 * Apple iOS interpreter entry: bundle path, search path, default DCB.
 *
 * Assets live at the .app root (flat bundle) and/or Documents. Relative
 * fopen is prefixed with the DCB directory in files_ios.c (SoRR
 * palettes/enemies/galsia.pal). Saves go to Documents via SDL_GetPrefPath.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "main_ios.h"
#include "files.h"
#include "files_ios.h"

static int ios_readable( const char * path )
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

static void ios_join( char * out, size_t out_sz, const char * root, const char * sub )
{
    size_t n;

    snprintf( out, out_sz, "%s", root );
    n = strlen( out );
    if ( n && out[ n - 1 ] != '/' && out_sz > n + 1 )
    {
        out[ n++ ] = '/';
        out[ n ] = '\0';
    }
    snprintf( out + n, out_sz - n, "%s", sub );
}

/* SoRR loads galsia.pal by basename; PATH must include palettes/enemies/. */
static void ios_add_dir_and_children( const char * dir )
{
    DIR * d;
    struct dirent * ent;
    struct stat st;
    char child[ __MAX_PATH ];

    file_addp( dir );

    d = opendir( dir );
    if ( !d )
        return;

    while ( ( ent = readdir( d ) ) )
    {
        if ( ent->d_name[0] == '.' )
            continue;
        ios_join( child, sizeof( child ), dir, ent->d_name );
        if ( stat( child, &st ) != 0 || !S_ISDIR( st.st_mode ) )
            continue;
        file_addp( child );
    }
    closedir( d );
}

static void ios_add_sorr_paths( const char * root )
{
    static const char * subs[] = {
        "palettes",
        "palettes/enemies",
        "palettes/players",
        "palettes/stages",
        "palettes/bonus",
        "palettes/boss",
        "palettes/misc",
        "mod",
        "data",
        "fpg",
        "fnt",
        "maps",
        "chars",
        "char",
        NULL
    };
    char path[ __MAX_PATH ];
    int i;

    if ( !root || !root[0] )
        return;

    file_addp( root );
    for ( i = 0 ; subs[i] ; i++ )
    {
        ios_join( path, sizeof( path ), root, subs[i] );
        file_addp( path );
    }
    ios_join( path, sizeof( path ), root, "palettes" );
    ios_add_dir_and_children( path );
}

static void ios_use_dcb( const char * dcb_path, const char * bundle, const char * docs )
{
    const char * root;

    file_ios_bind_root( dcb_path );
    root = file_ios_root();
    if ( docs && docs[0] )
        chdir( docs );
    else if ( root && root[0] )
        chdir( root );

    ios_add_sorr_paths( root );
    if ( docs && docs[0] && ( !root || strcmp( docs, root ) != 0 ) )
        ios_add_sorr_paths( docs );
    if ( bundle && bundle[0] && ( !root || strcmp( bundle, root ) != 0 ) )
        ios_add_sorr_paths( bundle );
    file_addp( "." );
    fprintf( stderr, "bgdi: data root %s\n", root ? root : "(none)" );
}

char * bgdi_ios_startup( int argc, char * argv[], int * standalone )
{
    static char dcb[ __MAX_PATH ];
    const char * base;
    const char * pref;

    SDL_SetHint( SDL_HINT_RENDER_DRIVER, "metal" );
    SDL_SetHint( SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight" );

    base = SDL_GetBasePath();
    pref = SDL_GetPrefPath( "bennugd64", "bgdi" );
    file_ios_set_roots( base, pref );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && argv[1][0] )
    {
        ios_use_dcb( argv[1], base, pref );
        return NULL;
    }

    if ( pref && pref[0] )
    {
        snprintf( dcb, sizeof( dcb ), "%smain.dcb", pref );
        if ( ios_readable( dcb ) )
        {
            fprintf( stderr, "bgdi: using %s\n", dcb );
            ios_use_dcb( dcb, base, pref );
            return dcb;
        }
    }

    if ( base && base[0] )
    {
        snprintf( dcb, sizeof( dcb ), "%smain.dcb", base );
        if ( ios_readable( dcb ) )
        {
            fprintf( stderr, "bgdi: using %s\n", dcb );
            ios_use_dcb( dcb, base, pref );
            return dcb;
        }
        fprintf( stderr, "bgdi: missing %s\n", dcb );
    }

    if ( ios_readable( "main.dcb" ) )
    {
        ios_use_dcb( "main.dcb", base, pref );
        return "main.dcb";
    }

    fprintf( stderr, "bgdi: main.dcb not found in the app bundle or Documents\n" );
    ios_use_dcb( NULL, base, pref );
    return NULL;
}
