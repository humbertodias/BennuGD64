/*
 * Default directory remove (POSIX rmdir / MSVC _rmdir).
 * Other targets replace this unit with dirs_wii.c.
 */

#if defined(TARGET_XBOX) || defined(__XBOX__) || defined(NXDK)
#include <direct.h>
#elif defined(WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "dirs_native.h"

int dir_native_rmdir( const char * path )
{
#if defined(TARGET_XBOX) || defined(__XBOX__) || defined(NXDK)
    return _rmdir( path );
#else
    return rmdir( path );
#endif
}
