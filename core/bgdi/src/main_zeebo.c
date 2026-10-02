/*
 * Zeebo interpreter entry. The console is BREW, so the ELF also exports
 * AEEMod_Load for a later elf2mod pack. crt0 still enters main().
 */

#include <stdio.h>
#include <string.h>

#include "main_zeebo.h"

extern int main( int argc, char * argv[] );

static int zeebo_file_exists( const char * path )
{
    FILE * fp = fopen( path, "rb" );
    if ( !fp ) return 0;
    fclose( fp );
    return 1;
}

char * bgdi_zeebo_startup( int argc, char * argv[], int * standalone )
{
    static const char * candidates[] = {
        "main.dcb",
        "SorR.dat",
        NULL
    };
    int i;

    if ( standalone ) *standalone = 1;

    if ( argc >= 2 && argv && argv[1] && zeebo_file_exists( argv[1] ) )
        return argv[1];

    for ( i = 0; candidates[i]; i++ )
    {
        if ( zeebo_file_exists( candidates[i] ) )
            return ( char * )candidates[i];
    }
    return NULL;
}

int AEEMod_Load( void * shell, void * helper, void ** mod )
{
    static char * argv[] = { "bgdi.elf", NULL };
    ( void )shell;
    ( void )helper;
    ( void )mod;
    return main( 1, argv );
}
