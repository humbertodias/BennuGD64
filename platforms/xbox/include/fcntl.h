/* Open flags for POSIX-path libretro VFS on nxdk. */
#ifndef __BENNUGD_XBOX_FCNTL_H
#define __BENNUGD_XBOX_FCNTL_H

#ifndef O_RDONLY
#define O_RDONLY 0
#endif
#ifndef O_WRONLY
#define O_WRONLY 1
#endif
#ifndef O_RDWR
#define O_RDWR 2
#endif
#ifndef O_CREAT
#define O_CREAT 0100
#endif
#ifndef O_TRUNC
#define O_TRUNC 01000
#endif
#ifndef O_APPEND
#define O_APPEND 02000
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

int open (const char *path, int flags, ...);

#endif
