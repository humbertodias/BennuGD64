/*
 * PlayStation 5 interpreter entry: pad, search path, default DCB.
 *
 * USB, /data and /user/homebrew drop-ins win over the PKG /app0 copy
 * and over a main.dcb sitting next to the ELF.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "main_ps5.h"
#include "files.h"
#include "files_ps5.h"
#include "ps5_log.h"
#include "ps5_platform.h"
#include "ps5_sce.h"
#include "../../../modules/libvideo/g_video_ps5.h"

static int ps5_suffix_ok( const char * name, const char * ext )
{
    size_t n, e;
    if ( !name || !ext ) return 0;
    n = strlen( name );
    e = strlen( ext );
    if ( n < e ) return 0;
    return strcasecmp( name + n - e, ext ) == 0;
}

static int ps5_arg_is_dcb( const char * a )
{
    return ps5_suffix_ok( a, ".dcb" ) || ps5_suffix_ok( a, ".dat" ) ||
           ps5_suffix_ok( a, ".bin" );
}

static int ps5_dcb_exists( const char * path )
{
    FILE * test;

    test = fopen( path, "rb" );
    if ( !test )
        return 0;
    fclose( test );
    return 1;
}

#ifndef BENNUGD_PS5_NATIVE_TITLE
static void ps5_chdir_from_argv0( const char * argv0 )
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

static void ps5_join( char * out, size_t out_sz, const char * root, const char * sub )
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

static void ps5_add_tree( const char * dir, int depth_left )
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
        ps5_join( child, sizeof( child ), dir, ent->d_name );
        if ( stat( child, &st ) != 0 || !S_ISDIR( st.st_mode ) )
            continue;
        ps5_add_tree( child, depth_left - 1 );
    }
    closedir( d );
}

static void ps5_add_search_paths( const char * root )
{
    ps5_add_tree( root, 4 );
}
#endif

static void ps5_use_dcb( const char * dcb_path )
{
#ifdef BENNUGD_PS5_NATIVE_TITLE
    file_ps5_bind_root( dcb_path );
    file_addp( file_ps5_root() );
    file_addp( "/app0/" );
    fprintf( stderr, "bgdi: data root %s\n", file_ps5_root() );
#else
    static const char * extra[] = {
        "/mnt/usb0/bennugd64/",
        "/data/bennugd64/",
        "/user/homebrew/bennugd64/",
        "/app0/",
        NULL
    };
    const char * root;
    int i;

    file_ps5_bind_root( dcb_path );
    root = file_ps5_root();
    chdir( root );
    ps5_add_search_paths( root );
    for ( i = 0 ; extra[i] ; i++ )
    {
        if ( strcmp( root, extra[i] ) != 0 )
            ps5_add_search_paths( extra[i] );
    }
    file_addp( "." );
    fprintf( stderr, "bgdi: data root %s\n", root );
#endif
}

static int ps5_pad_quit( void )
{
    ScePadData pad;

    if ( ps5_platform_pad_handle() < 0 )
        return 0;
    memset( &pad, 0, sizeof( pad ) );
    if ( ps5_platform_read_pad( &pad ) != 0 )
        return 0;
    return ( pad.buttons & ( SCE_PAD_BUTTON_CROSS | SCE_PAD_BUTTON_OPTIONS ) ) != 0;
}

static void ps5_missing_dcb( void )
{
    SDL_Surface * surface;
    int x, y;

#ifdef BENNUGD_PS5_NATIVE_TITLE
    ps5_log_write( "bgdi: game DCB not found" );
#else
    fprintf( stderr, "bgdi: main.dcb not found\n" );
#endif

    gr_video_ps5_module_initialize();
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
        if ( ps5_pad_quit() )
            exit( 0 );

        gr_video_ps5_present( surface );
        sceKernelUsleep( 16000 );
    }
}

char * bgdi_ps5_startup( int argc, char * argv[], int * standalone )
{
    static const char * bundled[] = {
#ifdef BENNUGD_PS5_NATIVE_TITLE
        "/app0/data/game.dcb",
        "/app0/main.dcb",
#else
        "/mnt/usb0/bennugd64/main.dcb",
        "/data/bennugd64/main.dcb",
        "/user/homebrew/bennugd64/main.dcb",
        "/app0/main.dcb",
        "main.dcb",
#endif
        NULL
    };
    int k;

    ps5_log_initialize();
    ps5_log_write( "bgdi: PS5 startup entered" );
    fprintf( stderr, "bgdi: ps5 start argc=%d\n", argc );
    if ( ps5_platform_initialize() != 0 )
        fprintf( stderr, "bgdi: controller unavailable; continuing\n" );
    ps5_log_write( "bgdi: PS5 platform initialized" );

    SDL_SetMainReady();

#ifdef BENNUGD_PS5_NATIVE_TITLE
    ps5_log_write( "bgdi: SDL main ready" );
#endif

#ifndef BENNUGD_PS5_NATIVE_TITLE
    if ( argc >= 1 && argv && argv[0] )
        ps5_chdir_from_argv0( argv[0] );

    file_addp( "/mnt/usb0/bennugd64/" );
    file_addp( "/data/bennugd64/" );
    file_addp( "/user/homebrew/bennugd64/" );
    file_addp( "/app0/" );
    file_addp( "." );
#endif

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && argv[1][0] && ps5_arg_is_dcb( argv[1] ) )
    {
        if ( ps5_dcb_exists( argv[1] ) )
        {
            ps5_use_dcb( argv[1] );
            return NULL;
        }
        fprintf( stderr, "bgdi: missing argv %s\n", argv[1] );
        ps5_missing_dcb();
    }

    for ( k = 0 ; bundled[k] ; k++ )
    {
#ifdef BENNUGD_PS5_NATIVE_TITLE
        ps5_log_write( bundled[k] );
#endif
        if ( ps5_dcb_exists( bundled[k] ) )
        {
            fprintf( stderr, "bgdi: using %s\n", bundled[k] );
            ps5_use_dcb( bundled[k] );
            return ( char * ) bundled[k];
        }
        fprintf( stderr, "bgdi: missing %s\n", bundled[k] );
    }

    ps5_missing_dcb();
    return NULL;
}
