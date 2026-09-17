#include "files_native.h"
#include "files_st.h"

struct RFILE;

extern struct RFILE * fopen_libretro( const char * filename, const char * mode );
extern int fseek_libretro( struct RFILE * stream, long int offset, int whence );
extern int rename_libretro( const char * old_filename, const char * new_filename );

int file_native_try_gzip( const char * filename )
{
    ( void ) filename;
    return 1;
}

FILE * file_native_fopen( const char * filename, const char * mode )
{
    return ( FILE * ) fopen_libretro( filename, mode );
}

int file_native_move( const char * source_file, const char * target_file )
{
    return rename_libretro( source_file, target_file );
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
    return fseek_libretro( ( struct RFILE * ) fp->fp, pos, where );
}

void file_ps2_bind_root( const char * dcb_path )
{
    ( void ) dcb_path;
}
