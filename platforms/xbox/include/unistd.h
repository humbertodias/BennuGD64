/* Minimal POSIX names for nxdk (underscore CRT in <direct.h>). */
#ifndef __BENNUGD_XBOX_UNISTD_H
#define __BENNUGD_XBOX_UNISTD_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

ssize_t read (int fd, void *buf, size_t n);
ssize_t write (int fd, const void *buf, size_t n);
off_t lseek (int fd, off_t offset, int whence);
int close (int fd);
int ftruncate (int fd, off_t length);
int fileno (FILE *stream);
int mkdir (const char *path, int mode);

#include <direct.h>

/* nxdk ships an empty unistd.h; keep a soft include_next if present. */
#if defined(__has_include_next)
#  if __has_include_next(<unistd.h>)
#    include_next <unistd.h>
#  endif
#endif

#ifndef chdir
#define chdir _chdir
#endif
#ifndef rmdir
#define rmdir _rmdir
#endif
#ifndef getcwd
#define getcwd _getcwd
#endif

#ifdef mkdir
#undef mkdir
#endif

#endif
