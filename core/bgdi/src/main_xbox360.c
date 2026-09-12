/*
 * Xbox 360 interpreter bootstrap for the open libXenon runtime.
 * Platform setup and storage discovery stay out of generic bgdi code.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <libfat/fat.h>
#include <console/console.h>
#include <usb/usbmain.h>
#include <xenos/xenos.h>
#include <xenon_soc/xenon_power.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "files.h"
#include "main_xbox360.h"

static int xbox360_file_exists( const char * path )
{
    FILE * fp = fopen( path, "rb" );
    if ( !fp ) return 0;
    fclose( fp );
    return 1;
}

static void xbox360_add_root( const char * dcb )
{
    char root[ __MAX_PATH ];
    char * slash;

    snprintf( root, sizeof( root ), "%s", dcb );
    slash = strrchr( root, '/' );
    if ( slash )
    {
        slash[1] = '\0';
        chdir( root );
        file_addp( root );
    }
    file_addp( "." );
}

char * bgdi_xbox360_startup( int argc, char * argv[], int * standalone )
{
    static const char * candidates[] = {
        "uda:/bennugd64/main.dcb",
        "uda:/bennugd64/SorR.dat",
        "udb:/bennugd64/main.dcb",
        "udb:/bennugd64/SorR.dat",
        "udc:/bennugd64/main.dcb",
        "udc:/bennugd64/SorR.dat",
        "sda:/bennugd64/main.dcb",
        "sda:/bennugd64/SorR.dat",
        "main.dcb",
        "SorR.dat",
        NULL
    };
    int i;

    xenos_init( VIDEO_MODE_AUTO );
    console_init();
    xenon_make_it_faster( XENON_SPEED_FULL );
    usb_init();
    usb_do_poll();

    if ( !fatInitDefault() )
        printf( "BennuGD64: no FAT storage found\n" );

    SDL_SetMainReady();
    if ( standalone ) *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && xbox360_file_exists( argv[1] ) )
    {
        xbox360_add_root( argv[1] );
        printf( "BennuGD64: loading %s\n", argv[1] );
        return argv[1];
    }

    for ( i = 0; candidates[i]; i++ )
    {
        if ( xbox360_file_exists( candidates[i] ) )
        {
            xbox360_add_root( candidates[i] );
            printf( "BennuGD64: loading %s\n", candidates[i] );
            return ( char * )candidates[i];
        }
    }

    printf( "\nBennuGD64: main.dcb not found\n" );
    printf( "Copy the game to uda:/bennugd64/main.dcb\n" );
    printf( "The USB drive must be FAT32 and the console must boot XeLL.\n" );
    for ( ;; ) usb_do_poll();
}
