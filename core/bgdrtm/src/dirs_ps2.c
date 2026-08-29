/*
 * PlayStation 2 cwd. getcwd/chdir after IOP reset hang (cwd is still host:).
 * fopen() uses the bound device via files_ps2; CD() only needs a string.
 */

#include <stdlib.h>
#include <string.h>

#include "dirs_ps2.h"

static char ps2_cwd[ 32 ] = "mass:/";

void dirs_ps2_set_cwd( const char * dir )
{
    size_t n;

    if ( !dir || !dir[0] )
        return;
    n = strlen( dir );
    if ( n >= sizeof( ps2_cwd ) )
        n = sizeof( ps2_cwd ) - 1;
    memcpy( ps2_cwd, dir, n );
    ps2_cwd[ n ] = '\0';
}

char * dirs_ps2_getcwd( char * buf, size_t size )
{
    size_t n = strlen( ps2_cwd ) + 1;

    if ( !buf )
    {
        char * s = ( char * ) malloc( n );
        if ( s )
            memcpy( s, ps2_cwd, n );
        return s;
    }
    if ( size == 0 )
        return NULL;
    strncpy( buf, ps2_cwd, size );
    buf[ size - 1 ] = '\0';
    return buf;
}

int dirs_ps2_chdir( const char * dir )
{
    ( void ) dir;
    return 0;
}
