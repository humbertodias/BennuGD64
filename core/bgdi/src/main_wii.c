/*
 * Nintendo Wii interpreter entry: libfat, search path, default DCB.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fat.h>
#include <gccore.h>
#include <ogc/usbmouse.h>
#include <wiikeyboard/keyboard.h>
#include <wiiuse/wpad.h>

#include "main_wii.h"
#include "files.h"
#include "dirs.h"

static void wii_banner( const char * msg )
{
    static int ready = 0 ;

    if ( !ready )
    {
        GXRModeObj * vmode ;
        void * xfb ;

        VIDEO_Init();
        vmode = VIDEO_GetPreferredMode( NULL );
        if ( !vmode ) vmode = &TVNtsc480IntDf ;
        xfb = MEM_K0_TO_K1( SYS_AllocateFramebuffer( vmode ) );
        CON_Init( xfb, 20, 20, vmode->fbWidth, vmode->xfbHeight,
                  vmode->fbWidth * VI_DISPLAY_PIX_SZ );
        VIDEO_Configure( vmode );
        VIDEO_SetNextFramebuffer( xfb );
        VIDEO_SetBlack( FALSE );
        VIDEO_Flush();
        VIDEO_WaitVSync();
        if ( vmode->viTVMode & VI_NON_INTERLACE ) VIDEO_WaitVSync();
        ready = 1 ;
    }
    printf( "BennuGD64: %s\n", msg );
    VIDEO_WaitVSync();
}

static int wii_file_readable( const char * path )
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

static void wii_add_search_paths( void )
{
    file_addp( "sd:/" );
    file_addp( "sd:/apps/bennugd64" );
    file_addp( "." );
}

static char * wii_pick_dcb( void )
{
    static const char * names[] = {
        "sd:/apps/bennugd64/main.dcb",
        NULL
    };
    char line[ 96 ];
    int i;

    for ( i = 0 ; names[i] ; i++ )
    {
        snprintf( line, sizeof( line ), "probe %s", names[i] );
        wii_banner( line );
        if ( wii_file_readable( names[i] ) )
        {
            snprintf( line, sizeof( line ), "dcb %s", names[i] );
            wii_banner( line );
            return ( char * ) names[i];
        }
    }
    return NULL;
}

static void wii_hold( void )
{
    for ( ;; )
    {
        WPAD_ScanPads();
        if ( WPAD_ButtonsDown( 0 ) & WPAD_BUTTON_HOME )
            break;
        VIDEO_WaitVSync();
    }
}

char * bgdi_wii_startup( int argc, char * argv[], int * standalone )
{
    char * dcb;

    ( void ) argv;

    wii_banner( "boot" );

    wii_banner( "wpad..." );
    WPAD_Init();
    WPAD_SetDataFormat( WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR );
    WPAD_SetVRes( WPAD_CHAN_ALL, 640, 480 );
    wii_banner( "wpad ok" );

    wii_banner( "mouse..." );
    MOUSE_Init();
    wii_banner( "mouse ok" );

    wii_banner( "kbd..." );
    KEYBOARD_Init( NULL );
    wii_banner( "kbd ok" );

    wii_banner( "fat..." );
    if ( fatInitDefault() )
        wii_banner( "fat ok" );
    else
        wii_banner( "fat fail" );

    SDL_SetMainReady();
    wii_add_search_paths();

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    dcb = wii_pick_dcb();
    if ( !dcb )
    {
        wii_banner( "no dcb (HOME=exit)" );
        wii_hold();
        return NULL;
    }
    return dcb;
}
