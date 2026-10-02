/*
 * newlib declares these POSIX calls and does not ship them for nosys.
 * The ELF keeps a static cwd so bgdi can resolve paths next to itself.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char zeebo_cwd[256] = "/";

char * getcwd( char * buf, size_t size )
{
    size_t n = strlen( zeebo_cwd ) + 1;

    if ( !buf )
    {
        if ( size < n ) size = n;
        buf = malloc( size );
        if ( !buf )
        {
            errno = ENOMEM;
            return 0;
        }
    }
    else if ( size < n )
    {
        errno = ERANGE;
        return 0;
    }
    memcpy( buf, zeebo_cwd, n );
    return buf;
}

int chdir( const char * path )
{
    if ( !path || !*path )
    {
        errno = ENOENT;
        return -1;
    }
    if ( path[0] == '/' )
    {
        strncpy( zeebo_cwd, path, sizeof( zeebo_cwd ) - 1 );
        zeebo_cwd[ sizeof( zeebo_cwd ) - 1 ] = '\0';
    }
    return 0;
}

int mkdir( const char * path, mode_t mode )
{
    ( void ) path;
    ( void ) mode;
    errno = ENOSYS;
    return -1;
}

int rmdir( const char * path )
{
    ( void ) path;
    errno = ENOSYS;
    return -1;
}

char * realpath( const char * path, char * resolved )
{
    if ( !path )
    {
        errno = EINVAL;
        return 0;
    }
    if ( !resolved )
    {
        resolved = malloc( 256 );
        if ( !resolved )
        {
            errno = ENOMEM;
            return 0;
        }
    }
    strncpy( resolved, path, 255 );
    resolved[255] = '\0';
    return resolved;
}
