/*
 * PlayStation 2 interpreter entry. SDL_MAIN_HANDLED so we run before any
 * IOP reset.
 *
 * Detect the ISO by fopen(MAIN.DCB) on cdrom0: BEFORE reset. PCSX2 often
 * does not pass cdrom0: in argv[0]; treating that as USB reset kills CDVD
 * and hangs on mass: — black screen. Disc boot must not load USB IRXs.
 *
 * PCSX2 File→Open with Host Filesystem keeps host: only if IOP is not reset.
 * Search host:main.dcb next to the ELF. ISO/cdrom boot keeps CDVD (no reset,
 * no SDL/cdfs before the DCB). Homebrew mkisofs images are CDs — do not force
 * DVD mode or fopen(cdrom0:\\MAIN.DCB;1) misses. USB boot still resets.
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
#include "dirs_ps2.h"

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
    static char host_elf[] = "host:bgdi.elf";
    static char host0_elf[] = "host0:bgdi.elf";
    static char host1_elf[] = "host1:bgdi.elf";

    if ( !dcb || !argv )
        return;
    if ( strstr( dcb, "mass" ) )
        argv[0] = mass_elf;
    else if ( strstr( dcb, "cdrom" ) || strstr( dcb, "cdfs" ) )
        argv[0] = cd_elf;
    else if ( strstr( dcb, "host0" ) )
        argv[0] = host0_elf;
    else if ( strstr( dcb, "host1" ) )
        argv[0] = host1_elf;
    else if ( strstr( dcb, "host" ) )
        argv[0] = host_elf;
}

static const char * const disc_dcb_names[] = {
    "MAIN.DCB",
    "main.dcb",
    "MAIN.DAT",
    "HELLO.DCB",
    NULL
};

static const char * const disc_dcb_fmts[] = {
    "cdrom0:\\%s;1",
    "cdrom0:\\\\%s;1",
    "cdrom0:%s;1",
    "cdrom0:\\%s",
    "cdrom0:/%s;1",
    "cdrom:%s;1",
    "cdfs:/%s",
    "cdfs:%s",
    NULL
};

static int ps2_ends_ci( const char * s, const char * suf )
{
    size_t n, m, i;
    unsigned char a, b;

    if ( !s || !suf )
        return 0;
    n = strlen( s );
    m = strlen( suf );
    if ( n < m )
        return 0;
    for ( i = 0 ; i < m ; i++ )
    {
        a = ( unsigned char ) s[ n - m + i ];
        b = ( unsigned char ) suf[ i ];
        if ( a >= 'A' && a <= 'Z' ) a = ( unsigned char )( a + 32 );
        if ( b >= 'A' && b <= 'Z' ) b = ( unsigned char )( b + 32 );
        if ( a != b )
            return 0;
    }
    return 1;
}

static int ps2_is_disc_path( const char * path )
{
    return path && ( strstr( path, "cdrom" ) || strstr( path, "cdfs" ) );
}

static int ps2_is_iso_name( const char * path )
{
    return ps2_ends_ci( path, ".iso" );
}

static int ps2_is_elf_name( const char * path )
{
    return ps2_ends_ci( path, ".elf" ) || ps2_ends_ci( path, ".elf;1" );
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

static int ps2_disc_fopen_any( void )
{
    char path[ 256 ];
    int i, j;

    for ( i = 0 ; disc_dcb_names[ i ] ; i++ )
    {
        for ( j = 0 ; disc_dcb_fmts[ j ] ; j++ )
        {
            snprintf( path, sizeof( path ), disc_dcb_fmts[ j ], disc_dcb_names[ i ] );
            if ( ps2_fopen_ok( path ) )
                return 1;
        }
    }
    return 0;
}

static void ps2_cdvd_mmode( int dvd )
{
    if ( dvd )
        sceCdMmode( SCECdPS2DVD );
    else
        sceCdMmode( SCECdPS2CD );
}

static void ps2_cdvd_setup( int wait )
{
    int type;

    sceCdInit( SCECdINoD );
    type = sceCdGetDiskType();
    /* A mkisofs homebrew ISO is a CD. Forcing DVD here makes fopen(cdrom0:) miss MAIN.DCB. */
    ps2_cdvd_mmode( type == SCECdPS2DVD );
    sceCdDiskReady( wait ? 0 : 1 );
}

static int ps2_is_usb_path( const char * path )
{
    return path && strncmp( path, "mass", 4 ) == 0;
}

static int ps2_is_host_device( const char * path )
{
    if ( !path || !path[0] )
        return 0;
    if ( strncmp( path, "host0:", 6 ) == 0 )
        return 1;
    if ( strncmp( path, "host1:", 6 ) == 0 )
        return 1;
    if ( strncmp( path, "host:", 5 ) == 0 )
        return 1;
    return 0;
}

static int ps2_is_win_path( const char * path )
{
    unsigned char a;

    if ( !path || !path[0] || path[1] != ':' )
        return 0;
    a = ( unsigned char ) path[0];
    if ( ( a >= 'A' && a <= 'Z' ) || ( a >= 'a' && a <= 'z' ) )
        return path[2] == '\0' || path[2] == '/' || path[2] == '\\';
    return 0;
}

static int ps2_is_host_boot( const char * path )
{
    if ( !path || !path[0] )
        return 0;
    if ( ps2_is_host_device( path ) )
        return 1;
    if ( path[0] == '/' )
        return 1;
    return ps2_is_win_path( path );
}

static int ps2_probe_disc( void )
{
    int type;

    ps2_cdvd_setup( 0 );
    type = sceCdGetDiskType();
    if ( type < 0x10 )
        return 0;
    if ( ps2_disc_fopen_any() )
        return 1;
    ps2_cdvd_mmode( 1 );
    sceCdDiskReady( 1 );
    if ( ps2_disc_fopen_any() )
        return 1;
    ps2_cdvd_mmode( 0 );
    sceCdDiskReady( 1 );
    return ps2_disc_fopen_any();
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

    if ( ps2_is_host_device( dcb_path ) )
    {
        file_addp( "host:/" );
        file_addp( "host:" );
        file_addp( "host0:/" );
        file_addp( "host0:" );
        file_addp( "host1:/" );
        file_addp( "host1:" );
        file_addp( "." );
        ps2_add_dir_of( dcb_path );
        return;
    }
    if ( ps2_is_disc_path( dcb_path ) )
    {
        file_addp( "cdfs:/" );
        file_addp( "cdrom0:/" );
        file_addp( "." );
        ps2_add_dir_of( dcb_path );
        return;
    }

    file_addp( "mass:/" );
    file_addp( "mass:" );
    file_addp( "cdfs:/" );
    file_addp( "cdrom0:/" );
    file_addp( "." );
    ps2_add_dir_of( dcb_path );
}

static void ps2_set_cwd_from( const char * path )
{
    if ( ps2_is_host_device( path ) )
    {
        if ( strncmp( path, "host0:", 6 ) == 0 )
            dirs_ps2_set_cwd( "host0:/" );
        else if ( strncmp( path, "host1:", 6 ) == 0 )
            dirs_ps2_set_cwd( "host1:/" );
        else
            dirs_ps2_set_cwd( "host:/" );
    }
    else if ( ps2_is_usb_path( path ) )
        dirs_ps2_set_cwd( "mass:/" );
    else if ( path && strstr( path, "cdfs" ) )
        dirs_ps2_set_cwd( "cdfs:/" );
    else if ( path && strstr( path, "cdrom" ) )
        dirs_ps2_set_cwd( "cdrom0:\\" );
}

static int ps2_take_dcb( const char * path, char * found, size_t found_sz,
                         int argc, char * argv[] )
{
    if ( !ps2_fopen_ok( path ) )
        return 0;
    file_ps2_bind_root( path );
    ps2_set_cwd_from( path );
    snprintf( found, found_sz, "%s", path );
    ps2_add_search_paths( path, argc, argv );
    return 1;
}

static int ps2_find_host_dcb( char * found, size_t found_sz, int argc, char * argv[] )
{
    static const char * host[] = {
        "host:main.dcb",
        "host:/main.dcb",
        "host:MAIN.DCB",
        "host:/MAIN.DCB",
        "host0:main.dcb",
        "host0:/main.dcb",
        "host0:MAIN.DCB",
        "host1:main.dcb",
        "host1:/main.dcb",
        NULL
    };
    int i;

    for ( i = 0 ; host[i] ; i++ )
    {
        if ( ps2_take_dcb( host[i], found, found_sz, argc, argv ) )
            return 1;
    }
    return 0;
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
    char path[ 256 ];
    int pass, i, j, n;

    for ( pass = 0 ; pass < 2 ; pass++ )
    {
        ps2_cdvd_mmode( pass == 1 );
        sceCdDiskReady( 0 );
        for ( n = 0 ; n < 16 ; n++ )
        {
            for ( i = 0 ; disc_dcb_names[ i ] ; i++ )
            {
                for ( j = 0 ; disc_dcb_fmts[ j ] ; j++ )
                {
                    snprintf( path, sizeof( path ), disc_dcb_fmts[ j ], disc_dcb_names[ i ] );
                    if ( ps2_take_dcb( path, found, found_sz, argc, argv ) )
                        return 1;
                }
            }
            sceCdSync( 0 );
        }
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
        SDL_RenderDebugText( renderer, 24, 88, "PCSX2: HostFS + main.dcb by ELF" );
        SDL_RenderDebugText( renderer, 24, 104, "USB: File>Open ELF + FAT32 .img" );
        SDL_RenderDebugText( renderer, 24, 120, "ISO: boot the disc (SYSTEM.CNF)" );
        if ( arg0 && arg0[0] )
        {
            SDL_RenderDebugText( renderer, 24, 152, "argv0:" );
            SDL_RenderDebugText( renderer, 24, 168, arg0 );
        }
        if ( cwd && cwd[0] )
        {
            SDL_RenderDebugText( renderer, 24, 192, "cwd:" );
            SDL_RenderDebugText( renderer, 24, 208, cwd );
        }
        SDL_RenderPresent( renderer );
        SDL_Delay( 16 );
    }
}

static int ps2_try_host_dir( const char * dir, char * found, size_t found_sz,
                             int argc, char * argv[] )
{
    static const char * names[] = { "main.dcb", "MAIN.DCB", NULL };
    char extra[ 256 ];
    int i;

    if ( !ps2_is_host_device( dir ) )
        return 0;
    for ( i = 0 ; names[ i ] ; i++ )
    {
        ps2_join( extra, sizeof( extra ), dir, names[ i ] );
        if ( ps2_take_dcb( extra, found, found_sz, argc, argv ) )
            return 1;
    }
    return 0;
}

static int ps2_try_host_beside( const char * path, char * found, size_t found_sz,
                                int argc, char * argv[] )
{
    char copy[ 256 ];
    char * slash;
    size_t n;

    if ( !ps2_is_host_device( path ) )
        return 0;
    n = strlen( path );
    if ( n >= sizeof( copy ) )
        n = sizeof( copy ) - 1;
    memcpy( copy, path, n );
    copy[ n ] = '\0';
    slash = strrchr( copy, '/' );
    if ( !slash )
        slash = strrchr( copy, '\\' );
    if ( slash )
        slash[1] = '\0';
    else
    {
        slash = strchr( copy, ':' );
        if ( slash )
            slash[1] = '\0';
    }
    return ps2_try_host_dir( copy, found, found_sz, argc, argv );
}

char * bgdi_ps2_startup( int argc, char * argv[], int * standalone )
{
    static char found[ 256 ];
    char cwd[ 256 ];
    int disc;
    int host;
    const char * arg0;

    if ( standalone )
        *standalone = 1;

    arg0 = ( argc > 0 && argv && argv[0] ) ? argv[0] : "";

    SDL_SetMainReady();
    SifInitRpc( 0 );
    __fdman_init();

    disc = 0;
    host = 0;
    cwd[0] = '\0';
    /* getcwd(cdrom0:) hangs. Skip it when File→Open already named the ISO/ELF on disc. */
    if ( !ps2_is_disc_path( arg0 ) && !ps2_is_iso_name( arg0 ) )
        getcwd( cwd, sizeof( cwd ) );
    if ( ps2_is_disc_path( arg0 ) || ps2_is_disc_path( cwd ) ||
         ps2_is_iso_name( arg0 ) || ps2_is_iso_name( cwd ) )
        disc = 1;
    else if ( ps2_is_host_device( arg0 ) ||
              ( ps2_is_host_boot( arg0 ) && ps2_is_elf_name( arg0 ) ) )
        host = 1;
    else if ( !ps2_is_usb_path( arg0 ) && !ps2_is_usb_path( cwd ) )
        disc = ps2_probe_disc();
    if ( !disc && !host && ( ps2_is_host_boot( arg0 ) || ps2_is_host_boot( cwd ) ) )
        host = 1;

    if ( disc )
    {
        /* No SDL / cdfs / memcard until MAIN.DCB opens — those unmount CDVD. */
        ps2_patches();
        ps2_cdvd_setup( 1 );

        if ( argc >= 2 && argv[1] && strstr( argv[1], ".dcb" ) )
        {
            if ( ps2_take_dcb( argv[1], found, sizeof( found ), argc, argv ) )
            {
                init_memcard_driver( 1 );
                return found;
            }
        }
        if ( ps2_find_disc_dcb( found, sizeof( found ), argc, argv ) )
        {
            init_memcard_driver( 1 );
            return found;
        }
        ps2_add_search_paths( "cdrom0:/", argc, argv );
        ps2_missing_dcb( cwd[0] ? cwd : NULL, arg0 );
        return NULL;
    }

    if ( host )
    {
        /* Keep IOP so PCSX2 HostFS stays mounted. USB drivers would replace it. */
        ps2_patches();
        ps2_splash( "BennuGD64: HostFS boot", arg0[0] ? arg0 : cwd );
        init_memcard_driver( 1 );

        if ( argc >= 2 && argv[1] && strstr( argv[1], ".dcb" ) )
        {
            if ( ps2_take_dcb( argv[1], found, sizeof( found ), argc, argv ) )
                return found;
        }
        if ( ps2_try_host_dir( cwd, found, sizeof( found ), argc, argv ) )
            return found;
        if ( ps2_try_host_beside( arg0, found, sizeof( found ), argc, argv ) )
            return found;
        if ( ps2_find_host_dcb( found, sizeof( found ), argc, argv ) )
            return found;

        ps2_add_search_paths( "host:/", argc, argv );
        ps2_missing_dcb( cwd[0] ? cwd : NULL, arg0 );
        return NULL;
    }

    ps2_reset_iop();
    ps2_patches();
    init_ps2_filesystem_driver();
    waitUntilDeviceIsReady( ( char * ) "mass:" );

    if ( argc >= 2 && argv[1] && strstr( argv[1], ".dcb" ) &&
         !ps2_is_host_boot( argv[1] ) )
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
