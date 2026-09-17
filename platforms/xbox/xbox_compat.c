/* CRT helpers missing from nxdk pdclib / disabled xboxrt stat. */
#include <dirent.h>
#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

#ifdef mkdir
#undef mkdir
#endif

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

int mkdir (const char *path, int mode)
{
    (void) mode;
    if (!path)
    {
        errno = EINVAL;
        return -1;
    }
    return _mkdir (path);
}

DIR *
opendir (const char *name)
{
    DIR *dir;
    char pattern[MAX_PATH];
    size_t n;

    if (!name || !*name)
    {
        errno = EINVAL;
        return NULL;
    }

    n = strlen (name);
    if (n + 3 >= sizeof (pattern))
    {
        errno = EINVAL;
        return NULL;
    }
    memcpy (pattern, name, n + 1);
    while (n > 0 && (pattern[n - 1] == '\\' || pattern[n - 1] == '/'))
        pattern[--n] = '\0';
    memcpy (pattern + n, "\\*", 3);

    dir = (DIR *) calloc (1, sizeof (*dir));
    if (!dir)
        return NULL;

    dir->handle = FindFirstFile (pattern, &dir->data);
    if (dir->handle == INVALID_HANDLE_VALUE)
    {
        free (dir);
        return NULL;
    }
    dir->first = 1;
    return dir;
}

struct dirent *
readdir (DIR *dirp)
{
    if (!dirp || dirp->done)
        return NULL;

    if (!dirp->first)
    {
        if (!FindNextFile (dirp->handle, &dirp->data))
        {
            dirp->done = 1;
            return NULL;
        }
    }
    dirp->first = 0;

    memset (dirp->ent.d_name, 0, sizeof (dirp->ent.d_name));
    strncpy (dirp->ent.d_name, dirp->data.cFileName, NAME_MAX);
    if (dirp->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        dirp->ent.d_type = DT_DIR;
    else
        dirp->ent.d_type = DT_REG;
    return &dirp->ent;
}

int
closedir (DIR *dirp)
{
    if (!dirp)
        return -1;
    if (dirp->handle != INVALID_HANDLE_VALUE)
        FindClose (dirp->handle);
    free (dirp);
    return 0;
}
