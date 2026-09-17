/* Prototypes libretro-common VFS needs; nxdk's unistd.h is empty. */

#ifndef BENNUGD_NXDK_VFS_POSIX_H
#define BENNUGD_NXDK_VFS_POSIX_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

#ifndef S_IRUSR
#define S_IRUSR 0400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0200
#endif
#ifndef S_IXUSR
#define S_IXUSR 0100
#endif

ssize_t read (int fd, void *buf, size_t n);
ssize_t write (int fd, const void *buf, size_t n);
off_t lseek (int fd, off_t offset, int whence);
int close (int fd);
int ftruncate (int fd, off_t length);
int fileno (FILE *stream);
int mkdir (const char *path, int mode);
int open (const char *path, int flags, ...);

#endif
