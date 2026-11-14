#include <dirent.h>
#include <errno.h>

DIR * opendir( const char * name )
{
    ( void ) name;
    errno = ENOSYS;
    return 0;
}

struct dirent * readdir( DIR * dirp )
{
    ( void ) dirp;
    errno = ENOSYS;
    return 0;
}

int closedir( DIR * dirp )
{
    ( void ) dirp;
    errno = ENOSYS;
    return -1;
}
