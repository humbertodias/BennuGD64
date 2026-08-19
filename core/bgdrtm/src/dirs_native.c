/*
 * Default directory remove (POSIX rmdir).
 * Other targets replace this unit with dirs_wii.c.
 */

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "dirs_native.h"

int dir_native_rmdir( const char * path )
{
    return rmdir( path );
}
