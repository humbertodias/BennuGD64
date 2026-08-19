/*
 * Nintendo Wii file I/O. Linked instead of files_native.c.
 *
 * Games (SoRR) open relative names such as palettes/enemies/galsia.pal.
 * Dolphin cwd is not the Homebrew folder, so fopen that as-is misses
 * sd:/apps/bennugd64/palettes/enemies/galsia.pal. Prefix here; leave
 * files.c unchanged. Skip gzopen (libfat + zlib fseeko(NULL)).
 */

#include <stdio.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"

#define WII_ROOT  "sd:/apps/bennugd64/"

static int has_device( const char * path )
{
    return path && strchr( path, ':' ) != NULL;
}

int file_native_try_gzip( const char * filename )
{
    ( void ) filename;
    return 0;
}

FILE * file_native_fopen( const char * filename, const char * mode )
{
    char path[ __MAX_PATH ];
    FILE * fp;

    if ( !filename || !filename[0] || !mode )
        return NULL;

    if ( has_device( filename ) )
        return fopen( filename, mode );

    if ( filename[0] == '.' && filename[1] == '/' )
        filename += 2;
    if ( filename[0] == '/' )
        filename++;

    snprintf( path, sizeof( path ), "%s%s", WII_ROOT, filename );
    fp = fopen( path, mode );
    if ( fp )
        return fp;

    return fopen( filename, mode );
}

int file_native_move( const char * source_file, const char * target_file )
{
    ( void ) source_file;
    ( void ) target_file;
    return -1;
}
