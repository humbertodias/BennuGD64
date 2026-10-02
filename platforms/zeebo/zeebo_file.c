/*
 * newlib syscalls for the Zeebo module.
 *
 * nosys fopen fails, and that made main() return -1 from AEEMod_Load before
 * the emulator had an applet. Zeebx serves the module directory through
 * IFileMgr, so open/read/seek go there once AEEMod_Load has stored the shell.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define AEECLSID_FILEMGR 0x01001003u
#define OFM_READWRITE    0x0002u
#define OFM_CREATE       0x0004u
#define OFM_APPEND       0x0008u
#define ZEEBO_FD_MAX     16

typedef int ( *zeebo_create_fn )( void * shell, uint32_t clsid, void ** out );
typedef void * ( *zeebo_open_fn )( void * fm, const char * path, uint32_t mode );
typedef uint32_t ( *zeebo_rw_fn )( void * file, void * buf, uint32_t count );
typedef int ( *zeebo_seek_fn )( void * file, uint32_t whence, int pos );
typedef int ( *zeebo_info_fn )( void * file, void * info );
typedef int ( *zeebo_release_fn )( void * obj );

typedef struct
{
    void * file;
    int pos;
    int size;
} zeebo_fd;

static void * zeebo_shell;
static void * zeebo_fm;
static zeebo_fd zeebo_fds[ZEEBO_FD_MAX];

void zeebo_set_shell( void * shell )
{
    zeebo_shell = shell;
}

static void ** zeebo_vt( void * obj )
{
    return *( void *** )obj;
}

static void * zeebo_filemgr( void )
{
    zeebo_create_fn create;
    void * out;

    if ( zeebo_fm || !zeebo_shell ) return zeebo_fm;
    create = ( zeebo_create_fn )( uintptr_t )zeebo_vt( zeebo_shell )[2];
    out = 0;
    if ( create( zeebo_shell, AEECLSID_FILEMGR, &out ) == 0 ) zeebo_fm = out;
    return zeebo_fm;
}

static int zeebo_alloc_fd( void )
{
    int i;

    for ( i = 0; i < ZEEBO_FD_MAX; i++ )
        if ( !zeebo_fds[i].file ) return i;
    return -1;
}

static void zeebo_putc( char c )
{
    unsigned int op = 3;
    unsigned int arg = ( unsigned int )( uintptr_t )&c;

    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "svc #0xab\n"
        :
        : "r"( op ), "r"( arg )
        : "r0", "r1", "lr", "memory"
    );
}

int _open( const char * path, int flags, int mode )
{
    void * fm;
    zeebo_open_fn open_file;
    zeebo_info_fn info;
    void * file;
    unsigned char raw[16];
    uint32_t size;
    uint32_t brew_mode = 0;
    int slot;
    int acc;

    ( void )mode;
    if ( !path || !path[0] )
    {
        errno = ENOENT;
        return -1;
    }
    fm = zeebo_filemgr();
    if ( !fm )
    {
        errno = ENOENT;
        return -1;
    }

    acc = flags & O_ACCMODE;
    if ( acc != O_RDONLY ) brew_mode |= OFM_READWRITE;
    if ( flags & O_CREAT ) brew_mode |= OFM_CREATE;
    if ( flags & O_APPEND ) brew_mode |= OFM_APPEND;

    slot = zeebo_alloc_fd();
    if ( slot < 0 )
    {
        errno = EMFILE;
        return -1;
    }

    open_file = ( zeebo_open_fn )( uintptr_t )zeebo_vt( fm )[2];
    file = open_file( fm, path, brew_mode );
    if ( !file )
    {
        errno = ENOENT;
        return -1;
    }

    memset( raw, 0, sizeof( raw ) );
    info = ( zeebo_info_fn )( uintptr_t )zeebo_vt( file )[9];
    info( file, raw );
    memcpy( &size, raw + 12, sizeof( size ) );

    zeebo_fds[slot].file = file;
    zeebo_fds[slot].pos = 0;
    zeebo_fds[slot].size = ( int )size;
    return slot + 3;
}

int _close( int fd )
{
    zeebo_release_fn release;
    int slot = fd - 3;

    if ( slot < 0 || slot >= ZEEBO_FD_MAX || !zeebo_fds[slot].file )
    {
        errno = EBADF;
        return -1;
    }
    release = ( zeebo_release_fn )( uintptr_t )zeebo_vt( zeebo_fds[slot].file )[1];
    release( zeebo_fds[slot].file );
    zeebo_fds[slot].file = 0;
    return 0;
}

int _read( int fd, char * buf, int len )
{
    zeebo_rw_fn read_file;
    uint32_t got;
    int slot = fd - 3;

    if ( slot < 0 || slot >= ZEEBO_FD_MAX || !zeebo_fds[slot].file || !buf || len < 0 )
    {
        errno = EBADF;
        return -1;
    }
    if ( len == 0 ) return 0;
    read_file = ( zeebo_rw_fn )( uintptr_t )zeebo_vt( zeebo_fds[slot].file )[3];
    got = read_file( zeebo_fds[slot].file, buf, ( uint32_t )len );
    zeebo_fds[slot].pos += ( int )got;
    return ( int )got;
}

int _write( int fd, const char * buf, int len )
{
    zeebo_rw_fn write_file;
    int i;
    int slot = fd - 3;

    if ( fd == 1 || fd == 2 )
    {
        if ( !buf || len < 0 ) return -1;
        for ( i = 0; i < len; i++ ) zeebo_putc( buf[i] );
        return len;
    }
    if ( slot < 0 || slot >= ZEEBO_FD_MAX || !zeebo_fds[slot].file || !buf || len < 0 )
    {
        errno = EBADF;
        return -1;
    }
    write_file = ( zeebo_rw_fn )( uintptr_t )zeebo_vt( zeebo_fds[slot].file )[5];
    return ( int )write_file( zeebo_fds[slot].file, ( void * )buf, ( uint32_t )len );
}

int _lseek( int fd, int ptr, int dir )
{
    zeebo_seek_fn seek;
    int slot = fd - 3;
    int next;

    if ( slot < 0 || slot >= ZEEBO_FD_MAX || !zeebo_fds[slot].file )
    {
        errno = EBADF;
        return -1;
    }
    if ( dir == SEEK_SET ) next = ptr;
    else if ( dir == SEEK_CUR ) next = zeebo_fds[slot].pos + ptr;
    else if ( dir == SEEK_END ) next = zeebo_fds[slot].size + ptr;
    else
    {
        errno = EINVAL;
        return -1;
    }
    if ( next < 0 ) next = 0;
    seek = ( zeebo_seek_fn )( uintptr_t )zeebo_vt( zeebo_fds[slot].file )[7];
    seek( zeebo_fds[slot].file, 0, next );
    zeebo_fds[slot].pos = next;
    return next;
}

int _fstat( int fd, struct stat * st )
{
    int slot = fd - 3;

    if ( !st )
    {
        errno = EFAULT;
        return -1;
    }
    memset( st, 0, sizeof( *st ) );
    if ( fd >= 0 && fd < 3 )
    {
        st->st_mode = S_IFCHR;
        return 0;
    }
    if ( slot < 0 || slot >= ZEEBO_FD_MAX || !zeebo_fds[slot].file )
    {
        errno = EBADF;
        return -1;
    }
    st->st_mode = S_IFREG | 0644;
    st->st_size = zeebo_fds[slot].size;
    return 0;
}

int _isatty( int fd )
{
    return fd >= 0 && fd < 3;
}

/* nosys grows the heap from `end`, inside the module. Zeebx only maps 1 MB
 * past that image, and the interpreter asks for 256 KB blocks. The guest
 * heap at 0x10000000 is 64 MB; newlib takes the upper 16 MB so it does not
 * sit on the first bytes BREW MALLOC hands out. */
#define ZEEBO_SBRK_BASE 0x13000000u
#define ZEEBO_SBRK_END  0x14000000u

void * _sbrk( int incr )
{
    static unsigned char * brk = ( unsigned char * )ZEEBO_SBRK_BASE;
    unsigned char * prev = brk;

    if ( incr < 0 || ( uintptr_t )( brk + incr ) > ZEEBO_SBRK_END )
    {
        errno = ENOMEM;
        return ( void * )-1;
    }
    brk += incr;
    return prev;
}
