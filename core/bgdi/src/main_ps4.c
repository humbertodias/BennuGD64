/*
 * PlayStation 4 interpreter entry: pad, search path, default DCB.
 *
 * Prefer USB/data drop-ins so a game replaces the bundled hello without
 * rebuilding the PKG. Paths follow OpenOrbis homebrew conventions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <orbis/Pad.h>
#include <orbis/UserService.h>

#include "main_ps4.h"
#include "files.h"
#include "files_ps4.h"

#ifndef BENNUGD_PS4_TITLE_ID
#define BENNUGD_PS4_TITLE_ID "BGD400001"
#endif

static int ps4_pad_handle = -1;

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

static void ps4_redirect_stdio( void )
{
    ps4_mkdir_p( "/data" );
    ps4_mkdir_p( "/data/bennugd64" );
    if ( !freopen( "/data/bennugd64/bgdi.log", "w", stdout ) )
        return;
    if ( !freopen( "/data/bennugd64/bgdi.log", "a", stderr ) )
        return;
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
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

static void ps4_add_dir_and_children( const char * dir )
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
        ps4_join( child, sizeof( child ), dir, ent->d_name );
        if ( stat( child, &st ) != 0 || !S_ISDIR( st.st_mode ) )
            continue;
        file_addp( child );
    }
    closedir( d );
}

static void ps4_add_sorr_paths( const char * root )
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

    file_addp( root );
    for ( i = 0 ; subs[i] ; i++ )
    {
        ps4_join( path, sizeof( path ), root, subs[i] );
        file_addp( path );
    }
    ps4_join( path, sizeof( path ), root, "palettes" );
    ps4_add_dir_and_children( path );
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
    ps4_add_sorr_paths( root );
    for ( i = 0 ; extra[i] ; i++ )
    {
        if ( strcmp( root, extra[i] ) != 0 )
            ps4_add_sorr_paths( extra[i] );
    }
    file_addp( "." );
    fprintf( stderr, "bgdi: data root %s\n", root );
}

static void ps4_pad_init( void )
{
    int32_t user = 0;

    sceUserServiceInitialize( NULL );
    scePadInit();
    if ( sceUserServiceGetInitialUser( &user ) != 0 )
        user = 0x1;
    ps4_pad_handle = scePadOpen( user, ORBIS_PAD_PORT_TYPE_STANDARD, 0, NULL );
}

static int ps4_pad_quit( void )
{
    OrbisPadData pad;

    if ( ps4_pad_handle < 0 )
        return 0;
    memset( &pad, 0, sizeof( pad ) );
    if ( scePadReadState( ps4_pad_handle, &pad ) != 0 )
        return 0;
    return ( pad.buttons & ( ORBIS_PAD_BUTTON_CROSS | ORBIS_PAD_BUTTON_OPTIONS ) ) != 0;
}

static void ps4_missing_dcb( void )
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Event event;

    fprintf( stderr, "bgdi: main.dcb not found\n" );

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

    window = SDL_CreateWindow( "bgdi", 1920, 1080, SDL_WINDOW_FULLSCREEN );
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

        if ( ps4_pad_quit() )
            exit( 0 );

        SDL_SetRenderDrawColor( renderer, 16, 16, 48, 255 );
        SDL_RenderClear( renderer );
        SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
        SDL_RenderDebugText( renderer, 16, 24, "BennuGD64: main.dcb not found" );
        SDL_RenderDebugText( renderer, 16, 56, "Copy a game DCB to:" );
        SDL_RenderDebugText( renderer, 16, 72, "/mnt/usb0/bennugd64/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 88, "or /data/bennugd64/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 120, "The PKG also looks in /app0/:" );
        SDL_RenderDebugText( renderer, 16, 136, "/app0/main.dcb" );
        SDL_RenderDebugText( renderer, 16, 168, "Compile with this tree's" );
        SDL_RenderDebugText( renderer, 16, 184, "ps4-host bgdc (not PC Bennu)." );
        SDL_RenderDebugText( renderer, 16, 216, "Log: /data/bennugd64/bgdi.log" );
        SDL_RenderDebugText( renderer, 16, 232, "CROSS / OPTIONS: quit" );
        SDL_RenderPresent( renderer );
        SDL_Delay( 16 );
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

    ps4_pad_init();
    ps4_redirect_stdio();
    fprintf( stderr, "bgdi: ps4 start argc=%d\n", argc );

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
