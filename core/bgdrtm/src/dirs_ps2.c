/*
 * PlayStation 2 cwd. getcwd/chdir after IOP reset hang (cwd is still host:).
 * fopen() uses mass:/ via files_ps2; CD() only needs a string.
 */

#include <stdlib.h>
#include <string.h>

#include "dirs_ps2.h"

static const char ps2_cwd[] = "mass:/";

char * dirs_ps2_getcwd( char * buf, size_t size )
{
    if ( !buf )
    {
        char * s = ( char * ) malloc( sizeof( ps2_cwd ) );
        if ( s )
            memcpy( s, ps2_cwd, sizeof( ps2_cwd ) );
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
