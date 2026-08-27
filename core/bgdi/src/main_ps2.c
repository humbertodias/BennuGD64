/*
 * PlayStation 2 interpreter entry: DCB search after SDL_RunApp() has mounted
 * filesystems. PCSX2 HostFS maps host: to the ELF folder — main.dcb must sit
 * next to bgdi.elf, and IOP reset is skipped so that mapping stays alive.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main_ps2.h"
#include "files.h"

static void ps2_join( char * out, size_t out_sz, const char * dir, const char * name )
{
    size_t n;

    if ( !dir || !dir[0] )
    {
        snprintf( out, out_sz, "%s", name );
        return;
    }
    n = strlen( dir );
    if ( dir[n - 1] == '/' || dir[n - 1] == '\\' || dir[n - 1] == ':' )
        snprintf( out, out_sz, "%s%s", dir, name );
    else
        snprintf( out, out_sz, "%s/%s", dir, name );
}

static int ps2_readable( const char * path )
{
    FILE * fp;

    if ( !path || !path[0] )
        return 0;
    fp = fopen( path, "rb" );
    if ( !fp )
        return 0;
    fclose( fp );
    return 1;
}

static void ps2_add_dir_of( const char * path )
{
    char copy[ 256 ];
    char * slash;
    size_t n;

    if ( !path || !path[0] )
        return;

    n = strlen( path );
    if ( n >= sizeof( copy ) )
        n = sizeof( copy ) - 1;
    memcpy( copy, path, n );
    copy[ n ] = '\0';

    slash = strrchr( copy, '/' );
    if ( !slash )
        slash = strrchr( copy, '\\' );
    if ( !slash )
        return;

    slash[1] = '\0';
    file_addp( copy );
    if ( copy[0] && strncmp( copy, "mass:", 5 ) != 0 && strncmp( copy, "cdrom0:", 7 ) != 0 )
        chdir( copy );
}

static void ps2_add_search_paths( const char * dcb_path, int argc, char * argv[] )
{
    char cwd[ 256 ];

    file_addp( "host:/" );
    file_addp( "host:" );
    file_addp( "." );
    file_addp( "mass:/" );
    file_addp( "cdrom0:/" );

    if ( getcwd( cwd, sizeof( cwd ) ) )
        file_addp( cwd );
    if ( argc > 0 && argv && argv[0] )
        ps2_add_dir_of( argv[0] );
    ps2_add_dir_of( dcb_path );
}

static void ps2_missing_dcb( const char * cwd )
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Event event;

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

    window = SDL_CreateWindow( "bgdi", 640, 448, SDL_WINDOW_FULLSCREEN );
    if ( !window )
        return;
    renderer = SDL_CreateRenderer( window, "PS2 gsKit" );
    if ( !renderer )
        renderer = SDL_CreateRenderer( window, NULL );
    if ( !renderer )
        return;

    for ( ;; )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_EVENT_QUIT )
                return;
        }
        SDL_SetRenderDrawColor( renderer, 16, 16, 48, 255 );
        SDL_RenderClear( renderer );
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderDebugText( renderer, 24, 64, "BennuGD64: main.dcb not found" );
        SDL_RenderDebugText( renderer, 24, 88, "Put main.dcb in the same folder as bgdi.elf" );
        SDL_RenderDebugText( renderer, 24, 112, "PCSX2: Settings > Emulation >" );
        SDL_RenderDebugText( renderer, 24, 128, "  Enable Host Filesystem" );
        SDL_RenderDebugText( renderer, 24, 152, "Then File > Open the ELF (not an ISO)" );
        if ( cwd && cwd[0] )
        {
            SDL_RenderDebugText( renderer, 24, 184, "cwd:" );
            SDL_RenderDebugText( renderer, 24, 200, cwd );
        }
        SDL_RenderPresent( renderer );
        SDL_Delay( 16 );
    }
}

char * bgdi_ps2_startup( int argc, char * argv[], int * standalone )
{
    static char found[ 256 ];
    static const char * bundled[] = {
        "host:/main.dcb",
        "host:/MAIN.DCB",
        "host:main.dcb",
        "host:MAIN.DCB",
        "host0:/main.dcb",
        "main.dcb",
        "cdrom0:/MAIN.DCB",
        "cdrom0:\\MAIN.DCB",
        "mass:/MAIN.DCB",
        "mass:/main.dcb",
        NULL
    };
    char cwd[ 256 ];
    char extra[ 256 ];
    int i;

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
    {
        ps2_add_search_paths( argv[1], argc, argv );
        return NULL;
    }

    cwd[0] = '\0';
    if ( getcwd( cwd, sizeof( cwd ) ) )
    {
        ps2_join( extra, sizeof( extra ), cwd, "main.dcb" );
        if ( ps2_readable( extra ) )
        {
            ps2_add_search_paths( extra, argc, argv );
            snprintf( found, sizeof( found ), "%s", extra );
            return found;
        }
        ps2_join( extra, sizeof( extra ), cwd, "MAIN.DCB" );
        if ( ps2_readable( extra ) )
        {
            ps2_add_search_paths( extra, argc, argv );
            snprintf( found, sizeof( found ), "%s", extra );
            return found;
        }
    }

    if ( argc > 0 && argv && argv[0] )
    {
        char dir[ 256 ];
        char * slash;
        size_t n = strlen( argv[0] );

        if ( n >= sizeof( dir ) )
            n = sizeof( dir ) - 1;
        memcpy( dir, argv[0], n );
        dir[ n ] = '\0';
        slash = strrchr( dir, '/' );
        if ( !slash )
            slash = strrchr( dir, '\\' );
        if ( slash )
        {
            slash[1] = '\0';
            ps2_join( extra, sizeof( extra ), dir, "main.dcb" );
            if ( ps2_readable( extra ) )
            {
                ps2_add_search_paths( extra, argc, argv );
                snprintf( found, sizeof( found ), "%s", extra );
                return found;
            }
        }
    }

    for ( i = 0 ; bundled[i] ; i++ )
    {
        if ( ps2_readable( bundled[i] ) )
        {
            ps2_add_search_paths( bundled[i], argc, argv );
            return ( char * ) bundled[i];
        }
    }

    ps2_add_search_paths( NULL, argc, argv );
    ps2_missing_dcb( cwd[0] ? cwd : NULL );
    return NULL;
}
