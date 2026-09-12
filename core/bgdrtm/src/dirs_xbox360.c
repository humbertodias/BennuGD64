/*
 * libXenon newlib does not expose rmdir(), although libfat supports the file
 * operations Bennu needs. Keep the unsupported operation contained here.
 */

#include <errno.h>

#include "dirs_native.h"

int dir_native_rmdir( const char * path )
{
    ( void ) path;
    errno = ENOSYS;
    return -1;
}
