/*
 * Dreamcast interpreter entry: KallistiOS init, /cd DCB search.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <kos.h>

#include "main_dc.h"
#include "files.h"

KOS_INIT_FLAGS(INIT_DEFAULT);

char * bgdi_dc_startup( int argc, char * argv[], int * standalone )
{
    static const char * bundled[] = {
        "/cd/main.dcb", "main.dcb", NULL
    };
    int k;
    FILE * test;

    ( void ) argv;

    SDL_SetMainReady();
    chdir( "/cd" );
    file_addp( "/cd/" );
    file_addp( "/pc/" );

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
