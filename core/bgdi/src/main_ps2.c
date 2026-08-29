/*
 * PlayStation 2 interpreter entry. SDL_MAIN_HANDLED so we run before any
 * IOP reset.
 *
 * Detect the ISO by fopen(MAIN.DCB) on cdrom0: BEFORE reset. PCSX2 often
 * does not pass cdrom0: in argv[0]; treating that as USB reset kills CDVD
 * and hangs on mass: — black screen. Disc boot must not load USB IRXs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iopcontrol.h>
#include <libcdvd.h>
#include <ps2_filesystem_driver.h>
#include <sbv_patches.h>
#include <sifrpc.h>

#include "main_ps2.h"
#include "files.h"
#include "files_native.h"

/* CRT would chdir(cdrom0:\\) from argv[0] before main — that hangs ISO boot. */
extern void __locks_init( void );
extern void __fdman_init( void );
extern char **environ;

static char *ps2_empty_env[] = { NULL };

void _libcglue_args_parse( int argc, char ** argv )
{
    ( void ) argc;
    ( void ) argv;
    environ = ps2_empty_env;
}

void _libcglue_init( void )
{
    environ = ps2_empty_env;
    __locks_init();
}

void _libcglue_deinit( void )
{
}

/* After IOP reset, newlib getenv() talks to a missing ENV device and hangs.
 * SDL_SetHint / SDL_Init both call getenv. */
char * getenv( const char * name )
{
    ( void ) name;
    return NULL;
}

int setenv( const char * name, const char * value, int overwrite )
{
    ( void ) name;
    ( void ) value;
    ( void ) overwrite;
    return -1;
}

int unsetenv( const char * name )
{
    ( void ) name;
    return -1;
}

int putenv( char * string )
{
    ( void ) string;
    return -1;
}

void bgdi_ps2_use_device_argv0( const char * dcb, char ** argv )
{
    static char mass_elf[] = "mass:/bgdi.elf";
    static char cd_elf[] = "cdrom0:\\BGDI.ELF;1";

    if ( !dcb || !argv )
        return;
    if ( strstr( dcb, "mass" ) )
        argv[0] = mass_elf;
    else if ( strstr( dcb, "cdrom" ) || strstr( dcb, "cdfs" ) )
        argv[0] = cd_elf;
}

static const char * const disc_dcbs[] = {
    "cdrom0:\\MAIN.DCB;1",
    "cdrom0:\\\\MAIN.DCB;1",
    "cdrom0:MAIN.DCB;1",
    "cdrom0:\\MAIN.DCB",
    "cdrom0:/MAIN.DCB;1",
    "cdfs:/MAIN.DCB",
    "cdfs:/main.dcb",
    NULL
};

static int ps2_is_disc_path( const char * path )
{
    return path && ( strstr( path, "cdrom" ) || strstr( path, "cdfs" ) );
}

static int ps2_fopen_ok( const char * path )
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

static int ps2_is_host_open( const char * path )
{
    if ( !path || !path[0] )
        return 0;
    if ( strncmp( path, "host:", 5 ) == 0 )
        return 1;
    if ( strncmp( path, "mass:", 5 ) == 0 )
        return 1;
    /* PCSX2 File→Open on macOS/Linux passes a host path. */
    if ( path[0] == '/' )
        return 1;
    return 0;
}

static int ps2_probe_disc( void )
{
    int i;
    int ready;
    int type;

    sceCdInit( SCECdINoD );
    sceCdMmode( SCECdPS2DVD );
    /* mode 1 = non-blocking. mode 0 waits forever when no disc (USB hang). */
    ready = sceCdDiskReady( 1 );
    type = sceCdGetDiskType();
    if ( type < 0x10 )
        return 0;
    for ( i = 0 ; disc_dcbs[ i ] ; i++ )
    {
        if ( ps2_fopen_ok( disc_dcbs[ i ] ) )
            return 1;
    }
    return 0;
}

static void ps2_patches( void )
{
    SifInitRpc( 0 );
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    sbv_patch_fileio();
}

static void ps2_reset_iop( void )
{
    SifInitRpc( 0 );
    while ( !SifIopReset( NULL, 0 ) )
    {
    }
    while ( !SifIopSync() )
    {
    }
}

static void ps2_splash( const char * line1, const char * line2 )
{
    static SDL_Window * window;
    static SDL_Renderer * renderer;

    if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    if ( !window )
    {
        window = SDL_CreateWindow( "bgdi", 640, 448, SDL_WINDOW_FULLSCREEN );
        if ( !window )
            return;
        renderer = SDL_CreateRenderer( window, "PS2 gsKit" );
        if ( !renderer )
            renderer = SDL_CreateRenderer( window, NULL );
        if ( !renderer )
            return;
    }
    if ( !renderer )
        return;
    SDL_SetRenderDrawColor( renderer, 16, 16, 48, 255 );
    SDL_RenderClear( renderer );
    SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
    if ( line1 )
        SDL_RenderDebugText( renderer, 24, 64, line1 );
    if ( line2 )
        SDL_RenderDebugText( renderer, 24, 88, line2 );
    SDL_RenderPresent( renderer );
}

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
}

static void ps2_add_search_paths( const char * dcb_path, int argc, char * argv[] )
{
    ( void ) argc;
    ( void ) argv;

    file_addp( "mass:/" );
    file_addp( "mass:" );
    file_addp( "cdfs:/" );
    file_addp( "cdrom0:/" );
    file_addp( "." );
    ps2_add_dir_of( dcb_path );
}

static int ps2_take_dcb( const char * path, char * found, size_t found_sz,
                         int argc, char * argv[] )
{
    if ( !ps2_fopen_ok( path ) )
        return 0;
    file_ps2_bind_root( path );
    snprintf( found, found_sz, "%s", path );
    ps2_add_search_paths( path, argc, argv );
    return 1;
}

static int ps2_find_usb_dcb( char * found, size_t found_sz, int argc, char * argv[] )
{
    static const char * usb[] = {
        "mass:/main.dcb",
        "mass:/MAIN.DCB",
        "mass:main.dcb",
        NULL
    };
    int i;

    for ( i = 0 ; usb[i] ; i++ )
    {
        if ( ps2_take_dcb( usb[i], found, found_sz, argc, argv ) )
            return 1;
    }
    return 0;
}

static int ps2_find_disc_dcb( char * found, size_t found_sz, int argc, char * argv[] )
{
    int i;

    for ( i = 0 ; disc_dcbs[ i ] ; i++ )
    {
        if ( ps2_take_dcb( disc_dcbs[ i ], found, found_sz, argc, argv ) )
            return 1;
    }
    return 0;
}

static void ps2_missing_dcb( const char * cwd, const char * arg0 )
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
        SDL_RenderDebugText( renderer, 24, 88, "USB: File>Open ELF + FAT32 .img" );
        SDL_RenderDebugText( renderer, 24, 104, "ISO: boot the disc (SYSTEM.CNF)" );
        if ( arg0 && arg0[0] )
        {
            SDL_RenderDebugText( renderer, 24, 136, "argv0:" );
            SDL_RenderDebugText( renderer, 24, 152, arg0 );
        }
        if ( cwd && cwd[0] )
        {
            SDL_RenderDebugText( renderer, 24, 176, "cwd:" );
            SDL_RenderDebugText( renderer, 24, 192, cwd );
        }
        SDL_RenderPresent( renderer );
        SDL_Delay( 16 );
    }
}

char * bgdi_ps2_startup( int argc, char * argv[], int * standalone )
{
    static char found[ 256 ];
    char cwd[ 256 ];
    char extra[ 256 ];
    int disc;
    const char * arg0;

    if ( standalone )
        *standalone = 1;

    arg0 = ( argc > 0 && argv && argv[0] ) ? argv[0] : "";
    cwd[0] = '\0';

    SDL_SetMainReady();
    SifInitRpc( 0 );
    __fdman_init();

    disc = 0;
    if ( ps2_is_disc_path( arg0 ) )
        disc = 1;
    else if ( !ps2_is_host_open( arg0 ) )
        disc = ps2_probe_disc();

    getcwd( cwd, sizeof( cwd ) );
    if ( !disc )
        disc = ps2_is_disc_path( cwd );

    if ( disc )
    {
        ps2_patches();
        sceCdInit( SCECdINoD );
        sceCdMmode( SCECdPS2DVD );
        sceCdDiskReady( 0 );
        ps2_splash( "BennuGD64: ISO boot", arg0[0] ? arg0 : cwd );
        init_memcard_driver( 1 );

        if ( argc >= 2 )
        {
            file_ps2_bind_root( argv[1] );
            ps2_add_search_paths( argv[1], argc, argv );
            return NULL;
        }
        if ( cwd[0] )
        {
            ps2_join( extra, sizeof( extra ), cwd, "MAIN.DCB;1" );
            if ( ps2_take_dcb( extra, found, sizeof( found ), argc, argv ) )
                return found;
        }
        if ( ps2_find_disc_dcb( found, sizeof( found ), argc, argv ) )
            return found;
        ps2_add_search_paths( NULL, argc, argv );
        ps2_missing_dcb( cwd[0] ? cwd : NULL, arg0 );
        return NULL;
    }

    ps2_reset_iop();
    ps2_patches();
    init_ps2_filesystem_driver();
    waitUntilDeviceIsReady( ( char * ) "mass:" );

    /* cwd/argv from File→Open are host: paths. IOP reset dropped HostFS —
     * fopen of those hangs after "mass ready". The DCB is on mass:. */
    if ( argc >= 2 && argv[1] && strstr( argv[1], ".dcb" ) &&
         !ps2_is_host_open( argv[1] ) )
    {
        if ( ps2_take_dcb( argv[1], found, sizeof( found ), argc, argv ) )
            return found;
    }
    if ( ps2_find_usb_dcb( found, sizeof( found ), argc, argv ) )
        return found;

    ps2_add_search_paths( NULL, argc, argv );
    ps2_missing_dcb( NULL, arg0 );
    return NULL;
}
