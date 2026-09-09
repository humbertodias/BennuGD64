/*
 * PlayStation 4 interpreter entry: pad, search path, default DCB.
 *
 * Prefer USB/data drop-ins so a game replaces the bundled hello without
 * rebuilding the PKG. Paths follow OpenOrbis homebrew conventions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <orbis/libkernel.h>

#include "main_ps4.h"
#include "files.h"
#include "files_ps4.h"
#include "ps4_platform.h"
#include "../../../modules/libvideo/g_video_ps4.h"

static int ps4_suffix_ok( const char * name, const char * ext )
{
    size_t n, e;
    if ( !name || !ext ) return 0;
    n = strlen( name );
    e = strlen( ext );
    if ( n < e ) return 0;
    return strcasecmp( name + n - e, ext ) == 0;
}

static int ps4_arg_is_dcb( const char * a )
{
    return ps4_suffix_ok( a, ".dcb" ) || ps4_suffix_ok( a, ".dat" ) ||
           ps4_suffix_ok( a, ".bin" );
}

static void ps4_mkdir_p( const char * path )
{
    mkdir( path, 0777 );
}

static int ps4_redirect_stdio( void )
{
    ps4_mkdir_p( "/data" );
    ps4_mkdir_p( "/data/bennugd64" );
    if ( !freopen( "/data/bennugd64/bgdi.log", "w", stderr ) )
        return -1;
    if ( !freopen( "/data/bennugd64/bgdi.log", "a", stdout ) )
    {
        fprintf( stderr, "bgdi: stdout redirect failed errno=%d\n", errno );
        return -1;
    }
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
    return 0;
}

static int ps4_dcb_exists( const char * path )
{
    FILE * test;

    test = fopen( path, "rb" );
    if ( !test )
        return 0;
    fclose( test );
    return 1;
}

static void ps4_chdir_from_argv0( const char * argv0 )
{
    char dir[ 512 ];
    char * slash;
    size_t n;

    if ( !argv0 || !argv0[0] )
        return;
    n = strlen( argv0 );
    if ( n >= sizeof( dir ) )
        n = sizeof( dir ) - 1;
    memcpy( dir, argv0, n );
    dir[ n ] = 0;
    slash = strrchr( dir, '/' );
    if ( !slash )
        return;
    *slash = 0;
    if ( dir[0] )
        chdir( dir );
}

static void ps4_join( char * out, size_t out_sz, const char * root, const char * sub )
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

static void ps4_add_tree( const char * dir, int depth_left )
{
    DIR * d;
    struct dirent * ent;
    struct stat st;
    char child[ __MAX_PATH ];

    file_addp( dir );
    if ( depth_left <= 0 )
        return;

    d = opendir( dir );
    if ( !d )
        return;

    while ( ( ent = readdir( d ) ) )
    {
        if ( ent->d_name[0] == '.' )
            continue;
        ps4_join( child, sizeof( child ), dir, ent->d_name );
        if ( stat( child, &st ) != 0 || !S_ISDIR( st.st_mode ) )
            continue;
        ps4_add_tree( child, depth_left - 1 );
    }
    closedir( d );
}

/* Register the DCB folder and nested dirs so basename opens resolve for any game. */
static void ps4_add_search_paths( const char * root )
{
    ps4_add_tree( root, 4 );
}

static void ps4_use_dcb( const char * dcb_path )
{
    static const char * extra[] = {
        "/mnt/usb0/bennugd64/",
        "/data/bennugd64/",
        "/app0/",
        NULL
    };
    const char * root;
    int i;

    file_ps4_bind_root( dcb_path );
    root = file_ps4_root();
    chdir( root );
    ps4_add_search_paths( root );
    for ( i = 0 ; extra[i] ; i++ )
    {
        if ( strcmp( root, extra[i] ) != 0 )
            ps4_add_search_paths( extra[i] );
    }
    file_addp( "." );
    fprintf( stderr, "bgdi: data root %s\n", root );
}

static int ps4_pad_quit( void )
{
    OrbisPadData pad;

    if ( ps4_platform_pad_handle() < 0 )
        return 0;
    memset( &pad, 0, sizeof( pad ) );
    if ( ps4_platform_read_pad( &pad ) != 0 )
        return 0;
    return ( pad.buttons & ( ORBIS_PAD_BUTTON_CROSS | ORBIS_PAD_BUTTON_OPTIONS ) ) != 0;
}

static void ps4_missing_dcb( void )
{
    SDL_Surface * surface;
    int x, y;

    fprintf( stderr, "bgdi: main.dcb not found\n" );

    gr_video_ps4_module_initialize();
    surface = SDL_CreateSurface( 960, 540, SDL_PIXELFORMAT_ARGB8888 );
    if ( !surface )
        for ( ;; ) sceKernelUsleep( 1000000 );
    for ( y = 0; y < surface->h; ++y )
    {
        uint32_t * row = ( uint32_t * )
            ( ( uint8_t * ) surface->pixels + y * surface->pitch );
        for ( x = 0; x < surface->w; ++x )
            row[ x ] = ( ( x / 32 + y / 32 ) & 1 )
                ? 0xff202060u : 0xff101030u;
    }

    for ( ;; )
    {
        if ( ps4_pad_quit() )
            exit( 0 );

        gr_video_ps4_present( surface );
        sceKernelUsleep( 16000 );
    }
}

char * bgdi_ps4_startup( int argc, char * argv[], int * standalone )
{
    static const char * bundled[] = {
        "/mnt/usb0/bennugd64/main.dcb",
        "/data/bennugd64/main.dcb",
        "/app0/main.dcb",
        "main.dcb",
        NULL
    };
    int k;

    ps4_redirect_stdio();
    fprintf( stderr, "bgdi: ps4 start argc=%d\n", argc );
    if ( ps4_platform_initialize() != 0 )
        fprintf( stderr, "bgdi: controller unavailable; continuing\n" );

    SDL_SetMainReady();

    if ( argc >= 1 && argv && argv[0] )
        ps4_chdir_from_argv0( argv[0] );

    file_addp( "/mnt/usb0/bennugd64/" );
    file_addp( "/data/bennugd64/" );
    file_addp( "/app0/" );
    file_addp( "." );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && argv[1][0] && ps4_arg_is_dcb( argv[1] ) )
    {
        if ( ps4_dcb_exists( argv[1] ) )
        {
            ps4_use_dcb( argv[1] );
            return NULL;
        }
        fprintf( stderr, "bgdi: missing argv %s\n", argv[1] );
        ps4_missing_dcb();
    }

    for ( k = 0 ; bundled[k] ; k++ )
    {
        if ( ps4_dcb_exists( bundled[k] ) )
        {
            fprintf( stderr, "bgdi: using %s\n", bundled[k] );
            ps4_use_dcb( bundled[k] );
            return ( char * ) bundled[k];
        }
        fprintf( stderr, "bgdi: missing %s\n", bundled[k] );
    }

    ps4_missing_dcb();
    return NULL;
}
