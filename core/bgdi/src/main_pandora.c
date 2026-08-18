/*
 * OpenPandora interpreter entry: standalone DCB next to the binary.
 */

#include <unistd.h>

#include "main_pandora.h"

char * bgdi_pandora_startup( int argc, char * argv[], int * standalone )
{
    ( void ) argv;

    if ( standalone )
        *standalone = 1;

    if ( argc >= 2 )
        return NULL;

    if ( access( "main.dcb", R_OK ) == 0 ) return "main.dcb";
    if ( access( "hello.dcb", R_OK ) == 0 ) return "hello.dcb";
    return NULL;
}
