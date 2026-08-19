/*
 * Default native file I/O (desktop / POSIX / Windows).
 * Other targets replace this unit with files_$PLATFORM.c.
 */

#include "files_native.h"

int file_native_try_gzip( const char * filename )
{
    ( void ) filename;
    return 1;
}

FILE * file_native_fopen( const char * filename, const char * mode )
{
    return fopen( filename, mode );
}

int file_native_move( const char * source_file, const char * target_file )
{
    return rename( source_file, target_file );
}
