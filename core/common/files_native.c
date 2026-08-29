/*
 * Default native file I/O (desktop / POSIX / Windows).
 * Other targets replace this unit with files_$PLATFORM.c.
 */

#include "files_native.h"
#include "files_st.h"

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

int file_native_size( file * fp )
{
    ( void ) fp;
    return -1;
}

int file_native_seek( file * fp, int pos, int where )
{
    if ( !fp || !fp->fp )
        return -1;
    return fseek( fp->fp, pos, where );
}

void file_ps2_bind_root( const char * dcb_path )
{
    ( void ) dcb_path;
}
