/* CRT helpers missing from nxdk pdclib / disabled xboxrt stat. */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

double atof( const char * nptr )
{
    if ( !nptr ) return 0.0;
    return strtod( nptr, NULL );
}

int stat( const char * path, struct stat * buf )
{
    DWORD attrs;

    if ( !path || !buf )
    {
        errno = EINVAL;
        return -1;
    }

    memset( buf, 0, sizeof( *buf ) );
    attrs = GetFileAttributesA( path );
    if ( attrs == INVALID_FILE_ATTRIBUTES )
    {
        errno = ENOENT;
        return -1;
    }

    if ( attrs & FILE_ATTRIBUTE_DIRECTORY )
        buf->st_mode = S_IFDIR;
    else
        buf->st_mode = S_IFREG;

    return 0;
}

int fstat( int fd, struct stat * buf )
{
    ( void )fd;
    if ( !buf )
    {
        errno = EINVAL;
        return -1;
    }
    memset( buf, 0, sizeof( *buf ) );
    buf->st_mode = S_IFREG;
    return 0;
}
