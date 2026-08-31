/*
 * PlayStation 3 interpreter entry: pad, search path, default DCB.
 *
 * Ignore extra argv unless it looks like a DCB. Prefer a USB/HDD drop-in
 * so a game replaces the bundled hello without rebuilding the PKG.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

#include <io/pad.h>

#include "main_ps3.h"
#include "files.h"

#ifndef BENNUGD_PS3_TITLE_ID
#define BENNUGD_PS3_TITLE_ID "BGD300001"
#endif

static int ps3_suffix_ok( const char * name, const char * ext )
{
    size_t n, e;
    if ( !name || !ext ) return 0;
    n = strlen( name );
    e = strlen( ext );
    if ( n < e ) return 0;
    return strcasecmp( name + n - e, ext ) == 0;
}

static int ps3_arg_is_dcb( const char * a )
{
    return ps3_suffix_ok( a, ".dcb" ) || ps3_suffix_ok( a, ".dat" ) ||
           ps3_suffix_ok( a, ".bin" );
}

static void ps3_mkdir_p( const char * path )
{
    mkdir( path, 0777 );
}

static void ps3_redirect_stdio( void )
{
    ps3_mkdir_p( "/dev_hdd0/tmp" );
    ps3_mkdir_p( "/dev_hdd0/tmp/bennugd64" );
    if ( !freopen( "/dev_hdd0/tmp/bennugd64/bgdi.log", "w", stdout ) )
        return;
    if ( !freopen( "/dev_hdd0/tmp/bennugd64/bgdi.log", "a", stderr ) )
        return;
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
}

static int ps3_dcb_exists( const char * path )
{
    FILE * test;

    test = fopen( path, "rb" );
    if ( !test )
        return 0;
    fclose( test );
    return 1;
}

static void ps3_chdir_from_argv0( const char * argv0 )
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

static void ps3_missing_dcb( void )
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Event event;
    padInfo info;
    padData pad;

    fprintf( stderr, "bgdi: main.dcb not found\n" );

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

    window = SDL_CreateWindow( "bgdi", 1280, 720, SDL_WINDOW_FULLSCREEN );
    if ( !window )
        exit( 0 );
    renderer = SDL_CreateRenderer( window, NULL );
    if ( !renderer )
        exit( 0 );

    SDL_SetRenderScale( renderer, 2.0f, 2.0f );

    for ( ;; )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_EVENT_QUIT )
                exit( 0 );
        }

        memset( &info, 0, sizeof( info ) );
        memset( &pad, 0, sizeof( pad ) );
        ioPadGetInfo( &info );
        if ( info.status[0] )
            ioPadGetData( 0, &pad );
        if ( pad.BTN_CROSS || pad.BTN_START || pad.BTN_SELECT )
            exit( 0 );

        SDL_SetRenderDrawColor( renderer, 16, 16, 48, 255 );
        SDL_RenderClear( renderer );
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderDebugText( renderer, 16, 24, "BennuGD64: main.dcb not found" );
        SDL_RenderDebugText( renderer, 16, 56, "Copy a game DCB to:" );
        SDL_RenderDebugText( renderer, 16, 72, "/dev_usb000/bennugd64/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 88, "or /dev_hdd0/tmp/bennugd64/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 120, "The PKG also looks in USRDIR:" );
        SDL_RenderDebugText( renderer, 16, 136, "/dev_hdd0/game/" BENNUGD_PS3_TITLE_ID "/USRDIR/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 168, "Compile with this tree's" );
        SDL_RenderDebugText( renderer, 16, 184, "ps3-host bgdc (not PC Bennu)." );
        SDL_RenderDebugText( renderer, 16, 216, "Log: /dev_hdd0/tmp/bennugd64/bgdi.log" );
        SDL_RenderDebugText( renderer, 16, 232, "CROSS / START / SELECT: quit" );
        SDL_RenderPresent( renderer );
        SDL_Delay( 16 );
    }
}

char * bgdi_ps3_startup( int argc, char * argv[], int * standalone )
{
    static const char * bundled[] = {
        "/dev_usb000/bennugd64/main.dcb",
        "/dev_hdd0/tmp/bennugd64/main.dcb",
        "/dev_hdd0/game/" BENNUGD_PS3_TITLE_ID "/USRDIR/main.dcb",
        "/app_home/main.dcb",
        "main.dcb",
        NULL
    };
    int k;

    ioPadInit( 7 );
    ps3_redirect_stdio();
    fprintf( stderr, "bgdi: ps3 start argc=%d\n", argc );

    SDL_SetMainReady();

    if ( argc >= 1 && argv && argv[0] )
        ps3_chdir_from_argv0( argv[0] );

    file_addp( "/dev_usb000/bennugd64/" );
    file_addp( "/dev_hdd0/tmp/bennugd64/" );
    file_addp( "/dev_hdd0/game/" BENNUGD_PS3_TITLE_ID "/USRDIR/" );
    file_addp( "/app_home/" );
    file_addp( "." );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && argv[1][0] && ps3_arg_is_dcb( argv[1] ) )
    {
        if ( ps3_dcb_exists( argv[1] ) )
            return NULL;
        fprintf( stderr, "bgdi: missing argv %s\n", argv[1] );
        ps3_missing_dcb();
    }

    for ( k = 0 ; bundled[k] ; k++ )
    {
        if ( ps3_dcb_exists( bundled[k] ) )
        {
            fprintf( stderr, "bgdi: using %s\n", bundled[k] );
            return ( char * ) bundled[k];
        }
        fprintf( stderr, "bgdi: missing %s\n", bundled[k] );
    }

    ps3_missing_dcb();
    return NULL;
}

/* PSL1GHT newlib declares sysconf in unistd.h but does not define it.
 * SDL_cpuinfo uses it when HAVE_SYSCONF was a false-positive try_compile. */
long sysconf( int name )
{
#ifdef _SC_NPROCESSORS_ONLN
    if ( name == _SC_NPROCESSORS_ONLN )
        return 2;
#endif
#ifdef _SC_NPROCESSORS_CONF
    if ( name == _SC_NPROCESSORS_CONF )
        return 2;
#endif
#ifdef _SC_PAGESIZE
    if ( name == _SC_PAGESIZE )
        return 4096;
#endif
#ifdef _SC_PAGE_SIZE
    if ( name == _SC_PAGE_SIZE )
        return 4096;
#endif
#ifdef _SC_PHYS_PAGES
    if ( name == _SC_PHYS_PAGES )
        return ( 256 * 1024 * 1024 ) / 4096;
#endif
    (void) name;
    return -1;
}
