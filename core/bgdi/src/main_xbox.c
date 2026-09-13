/*
 * Original Xbox interpreter bootstrap for the open nxdk runtime.
 * Storage discovery and DCB search stay out of generic bgdi code.
 */

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
#include <hal/video.h>
#include <hal/debug.h>
#include <nxdk/mount.h>
#include <nxdk/path.h>

#include "files.h"
#include "main_xbox.h"

static int xbox_file_exists( const char * path )
{
    FILE * fp = fopen( path, "rb" );
    if ( !fp ) return 0;
    fclose( fp );
    return 1;
}

/* Point D: at the folder that contains this XBE (DVD root or HDD game dir).
 * xemu often already has D: mounted from the dashboard; automount_d then skips. */
static void xbox_bind_d_to_xbe_dir( void )
{
    char nt_path[ MAX_PATH ];
    char * slash;

    nxGetCurrentXbeNtPath( nt_path );
    slash = strrchr( nt_path, '\\' );
    if ( !slash ) return;
    slash[1] = '\0';

    if ( nxIsDriveMounted( 'D' ) )
        nxUnmountDrive( 'D' );
    nxMountDrive( 'D', nt_path );
    debugPrint( "D: -> %s\n", nt_path );
}

static void xbox_add_root( const char * dcb )
{
    char root[ __MAX_PATH ];
    char * slash;
    size_t n;

    snprintf( root, sizeof( root ), "%s", dcb );
    slash = strrchr( root, '\\' );
    if ( !slash ) slash = strrchr( root, '/' );
    if ( slash )
    {
        slash[1] = '\0';
        _chdir( root );
        file_addp( root );
        n = strlen( root );
        if ( n > 1 && ( root[n - 1] == '\\' || root[n - 1] == '/' ) )
        {
            root[n - 1] = '\0';
            file_addp( root );
        }
    }
    file_addp( "." );
    file_addp( "D:\\" );
    file_addp( "D:\\bennugd64" );
    file_addp( "D:\\bennugd64\\" );
    file_addp( "E:\\bennugd64" );
    file_addp( "E:\\bennugd64\\" );
}

char * bgdi_xbox_startup( int argc, char * argv[], int * standalone )
{
    static const char * candidates[] = {
        "D:\\main.dcb",
        "D:\\SorR.dat",
        "D:\\sorr.dat",
        "D:\\bennugd64\\main.dcb",
        "D:\\bennugd64\\SorR.dat",
        "D:\\bennugd64\\sorr.dat",
        "E:\\bennugd64\\main.dcb",
        "E:\\bennugd64\\SorR.dat",
        "E:\\bennugd64\\sorr.dat",
        "E:\\main.dcb",
        "main.dcb",
        "SorR.dat",
        "sorr.dat",
        NULL
    };
    int i;

    XVideoSetMode( 640, 480, 32, REFRESH_DEFAULT );
    debugClearScreen();
    debugPrint( "BennuGD64 Xbox (nxdk)\n" );
    xbox_bind_d_to_xbe_dir();

    if ( standalone ) *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && xbox_file_exists( argv[1] ) )
    {
        xbox_add_root( argv[1] );
        debugPrint( "loading %s\n", argv[1] );
        return argv[1];
    }

    for ( i = 0; candidates[i]; i++ )
    {
        if ( xbox_file_exists( candidates[i] ) )
        {
            xbox_add_root( candidates[i] );
            debugPrint( "loading %s\n", candidates[i] );
            return ( char * )candidates[i];
        }
    }

    bgdi_xbox_hang( "main.dcb not found\nDisc: next to default.xbe\nHDD: E:\\bennugd64\\" );
    return NULL;
}

void bgdi_xbox_hang( const char * msg )
{
    if ( msg && *msg ) debugPrint( "\n%s\n", msg );
    debugPrint( "(halted)\n" );
    for ( ;; ) Sleep( 1000 );
}
