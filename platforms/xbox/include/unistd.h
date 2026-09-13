/* Minimal POSIX names for nxdk (underscore CRT in <direct.h>). */
#ifndef __BENNUGD_XBOX_UNISTD_H
#define __BENNUGD_XBOX_UNISTD_H

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
#ifndef mkdir
#define mkdir _mkdir
#endif
#ifndef getcwd
#define getcwd _getcwd
#endif

#endif
