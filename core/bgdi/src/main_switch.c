/*
 * Nintendo Switch interpreter entry: RomFS, nxlink, default DCB search.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <switch.h>

#include "main_switch.h"
#include "files.h"

char * bgdi_switch_startup( int argc, char * argv[], int * standalone )
{
    static const char * bundled[] = {
        "romfs:/main.dcb", "main.dcb", NULL
    };
    int k;
    FILE * test;

    ( void ) argv;

    SDL_SetMainReady();
    romfsInit();
    socketInitializeDefault();
    nxlinkStdio();
    file_addp( "romfs:/" );
    file_addp( "sdmc:/switch/bennugd64" );
    chdir( "romfs:/" );

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    for ( k = 0 ; bundled[k] ; k++ )
    {
        test = fopen( bundled[k], "rb" );
        if ( test )
        {
            fclose( test );
            return ( char * ) bundled[k];
        }
    }

    return NULL;
}
