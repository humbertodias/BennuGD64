/* CRT helpers missing from nxdk pdclib / disabled xboxrt stat. */
#include <dirent.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>
#include <winextras.h>

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

#define NXDK_FD_MAX 64

static HANDLE nxdk_fds[NXDK_FD_MAX];

static HANDLE
nxdk_fd_handle (int fd)
{
    if (fd < 0 || fd >= NXDK_FD_MAX || !nxdk_fds[fd])
        return INVALID_HANDLE_VALUE;
    return nxdk_fds[fd];
}

static int
nxdk_fd_alloc (HANDLE h)
{
    int i;

    if (h == NULL || h == INVALID_HANDLE_VALUE)
        return -1;
    for (i = 3; i < NXDK_FD_MAX; i++)
    {
        if (!nxdk_fds[i])
        {
            nxdk_fds[i] = h;
            return i;
        }
    }
    CloseHandle (h);
    errno = EINVAL;
    return -1;
}

int
open (const char *path, int flags, ...)
{
    DWORD access = GENERIC_READ;
    DWORD disp = OPEN_EXISTING;
    HANDLE h;

    if (!path)
    {
        errno = EINVAL;
        return -1;
    }

    if (flags & O_RDWR)
        access = GENERIC_READ | GENERIC_WRITE;
    else if (flags & O_WRONLY)
        access = GENERIC_WRITE;

    if (flags & O_CREAT)
        disp = (flags & O_TRUNC) ? CREATE_ALWAYS : OPEN_ALWAYS;
    else if (flags & O_TRUNC)
        disp = TRUNCATE_EXISTING;

    h = CreateFile (path, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    disp, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        errno = ENOENT;
        return -1;
    }
    if (flags & O_APPEND)
        SetFilePointer (h, 0, NULL, FILE_END);
    return nxdk_fd_alloc (h);
}

int
close (int fd)
{
    HANDLE h = nxdk_fd_handle (fd);
    if (h == INVALID_HANDLE_VALUE)
    {
        errno = EINVAL;
        return -1;
    }
    CloseHandle (h);
    nxdk_fds[fd] = NULL;
    return 0;
}

ssize_t
read (int fd, void *buf, size_t n)
{
    DWORD got = 0;
    HANDLE h = nxdk_fd_handle (fd);

    if (h == INVALID_HANDLE_VALUE || !buf)
    {
        errno = EINVAL;
        return -1;
    }
    if (!ReadFile (h, buf, (DWORD) n, &got, NULL))
        return -1;
    return (ssize_t) got;
}

ssize_t
write (int fd, const void *buf, size_t n)
{
    DWORD got = 0;
    HANDLE h = nxdk_fd_handle (fd);

    if (h == INVALID_HANDLE_VALUE || !buf)
    {
        errno = EINVAL;
        return -1;
    }
    if (!WriteFile (h, buf, (DWORD) n, &got, NULL))
        return -1;
    return (ssize_t) got;
}

off_t
lseek (int fd, off_t offset, int whence)
{
    DWORD method = FILE_BEGIN;
    DWORD pos;
    HANDLE h = nxdk_fd_handle (fd);

    if (h == INVALID_HANDLE_VALUE)
    {
        errno = EINVAL;
        return (off_t) -1;
    }
    if (whence == SEEK_CUR)
        method = FILE_CURRENT;
    else if (whence == SEEK_END)
        method = FILE_END;
    pos = SetFilePointer (h, (LONG) offset, NULL, method);
    if (pos == INVALID_SET_FILE_POINTER)
        return (off_t) -1;
    return (off_t) pos;
}

int
ftruncate (int fd, off_t length)
{
    HANDLE h = nxdk_fd_handle (fd);

    if (h == INVALID_HANDLE_VALUE)
    {
        errno = EINVAL;
        return -1;
    }
    if (SetFilePointer (h, (LONG) length, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return -1;
    if (!SetEndOfFile (h))
        return -1;
    return 0;
}

int
fileno (FILE *stream)
{
    (void) stream;
    return -1;
}
