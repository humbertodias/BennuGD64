/*
 * Wii directory remove. libfat rmdir is unreliable; use remove().
 * Compiled only into the wii-powerpc build (instead of dirs_native.c).
 */

#include <stdio.h>

#include "dirs_native.h"

int dir_native_rmdir( const char * path )
{
    return remove( path );
}
