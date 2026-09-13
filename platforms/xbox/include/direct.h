/* nxdk CRT uses underscore names; Bennu expects POSIX/MSVC aliases. */
#ifndef __BENNUGD_XBOX_DIRECT_H
#define __BENNUGD_XBOX_DIRECT_H

#include_next <direct.h>

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
