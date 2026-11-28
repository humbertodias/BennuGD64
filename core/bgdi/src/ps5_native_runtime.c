/* libc interfaces used by Bennu and SDL that the native title cannot import. */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <SDL3/SDL_stdinc.h>

#include "ps5_sce.h"

/* The PS5 kernel exports getdents to titles through libkernel. */
extern int getdents( int fd, char * buffer, int size );

int __wrap_stat( const char * path, struct stat * info )
{
    int fd = open( path, O_RDONLY );
    if ( fd < 0 )
        return -1;

    int result = fstat( fd, info );
    int saved_errno = errno;
    close( fd );
    if ( result < 0 )
        errno = saved_errno;
    return result;
}

_Noreturn void __wrap__exit( int status )
{
    (void) status;
    sceSystemServiceLoadExec( "exit", NULL );
    for ( ;; )
        sceKernelUsleep( 1000000 );
}

_Noreturn void __wrap_exit( int status )
{
    fflush( NULL );
    __wrap__exit( status );
}

_Noreturn void __wrap__Exit( int status )
{
    __wrap__exit( status );
}

typedef struct
{
    int fd;
    size_t offset;
    size_t used;
    _Alignas(struct dirent) char data[8192];
} NativeDirectory;

DIR * opendir( const char * path )
{
    int fd = open( path, O_RDONLY | O_DIRECTORY );
    if ( fd < 0 )
        return NULL;

    NativeDirectory * dir = calloc( 1, sizeof( *dir ) );
    if ( !dir )
    {
        close( fd );
        errno = ENOMEM;
        return NULL;
    }
    dir->fd = fd;
    return (DIR *) dir;
}

struct dirent * readdir( DIR * stream )
{
    NativeDirectory * dir = (NativeDirectory *) stream;
    const size_t header = offsetof( struct dirent, d_name );

    if ( dir->offset == dir->used )
    {
        int count = getdents( dir->fd, dir->data, sizeof( dir->data ) );
        if ( count <= 0 )
            return NULL;
        dir->used = count;
        dir->offset = 0;
    }

    size_t available = dir->used - dir->offset;
    struct dirent * entry = (struct dirent *) (dir->data + dir->offset);
    if ( available <= header || entry->d_reclen > available ||
         entry->d_reclen <= header + (size_t) entry->d_namlen )
    {
        dir->offset = dir->used;
        errno = EIO;
        return NULL;
    }

    dir->offset += entry->d_reclen;
    return entry;
}

int closedir( DIR * stream )
{
    NativeDirectory * dir = (NativeDirectory *) stream;
    int result = close( dir->fd );
    free( dir );
    return result;
}

/* The title's libc has no getcwd. Walk parent entries without changing cwd. */
static int ps5_native_prepend_dir( const char * parent, const struct stat * child,
    char * path, size_t * start )
{
    DIR * dir = opendir( parent );
    if ( !dir )
        return -1;

    struct dirent * entry;
    char candidate[PATH_MAX];
    while ( (entry = readdir( dir )) )
    {
        if ( entry->d_type == DT_LNK || !strcmp( entry->d_name, "." ) ||
             !strcmp( entry->d_name, ".." ) )
            continue;

        int length = snprintf( candidate, sizeof( candidate ), "%s/%s", parent, entry->d_name );
        struct stat info;
        if ( length < 0 || (size_t) length >= sizeof( candidate ) ||
             stat( candidate, &info ) != 0 || info.st_dev != child->st_dev ||
             info.st_ino != child->st_ino )
            continue;

        size_t name_length = entry->d_namlen;
        if ( name_length + 1 > *start )
        {
            closedir( dir );
            errno = ERANGE;
            return -1;
        }
        *start -= name_length + 1;
        path[*start] = '/';
        memcpy( path + *start + 1, entry->d_name, name_length );
        closedir( dir );
        return 0;
    }

    closedir( dir );
    errno = ENOENT;
    return -1;
}

char * getcwd( char * buffer, size_t size )
{
    char parent[PATH_MAX] = ".";
    char path[PATH_MAX];
    size_t start = sizeof( path ) - 1;
    path[start] = '\0';

    struct stat child;
    if ( stat( ".", &child ) != 0 )
        return NULL;

    for ( size_t depth = 1; ; depth += 3 )
    {
        if ( depth + sizeof( "/.." ) > sizeof( parent ) )
        {
            errno = ERANGE;
            return NULL;
        }
        memcpy( parent + depth, "/..", sizeof( "/.." ) );
        struct stat ancestor;
        if ( stat( parent, &ancestor ) != 0 )
            return NULL;
        if ( child.st_dev == ancestor.st_dev && child.st_ino == ancestor.st_ino )
            break;
        if ( ps5_native_prepend_dir( parent, &child, path, &start ) != 0 )
            return NULL;
        child = ancestor;
    }

    if ( start == sizeof( path ) - 1 )
        path[--start] = '/';
    size_t length = sizeof( path ) - start;
    if ( !buffer && !size )
        size = length;
    if ( size < length )
    {
        errno = ERANGE;
        return NULL;
    }
    if ( !buffer && !(buffer = malloc( size )) )
        return NULL;
    memcpy( buffer, path + start, length );
    return buffer;
}

char * getenv( const char * name )
{
    /* SDL asks for its override library while holding the DynAPI startup
       lock. Calling SDL_getenv_unsafe here would enter the same lock again. */
    if ( strcmp( name, "SDL3_DYNAMIC_API" ) == 0 )
        return NULL;

    return (char *) SDL_getenv_unsafe( name );
}

/* SDL falls back when Unix path and saved-user/group probes are unavailable. */
ssize_t readlink( const char * path, char * buffer, size_t size )
{
    (void) path;
    (void) buffer;
    (void) size;
    errno = ENOSYS;
    return -1;
}

int getresuid( uid_t * real, uid_t * effective, uid_t * saved )
{
    (void) real;
    (void) effective;
    (void) saved;
    errno = ENOSYS;
    return -1;
}

int getresgid( gid_t * real, gid_t * effective, gid_t * saved )
{
    (void) real;
    (void) effective;
    (void) saved;
    errno = ENOSYS;
    return -1;
}

FILE * __stdinp;
FILE * __stdoutp;
FILE * __stderrp;

__attribute__((constructor))
static void ps5_native_initialize_streams( void )
{
    sceKernelDebugOutText( 0, "bgdi: native constructor entered\n" );
    __stdinp = fopen( "/dev/null", "r" );
    __stdoutp = fopen( "/dev/null", "w" );
    __stderrp = fopen( "/dev/null", "w" );
}
