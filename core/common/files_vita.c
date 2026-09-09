/*
 * PlayStation Vita file I/O. Linked instead of files_native.c.
 *
 * Games open relative names (subdir/file or basename). Vita3K honours POSIX
 * cwd after chdir("app0:/"); real hardware often does not, so a bare fopen
 * misses files next to the DCB under ux0:/data/bennugd64/ or app0:/. Prefix
 * with the DCB directory here; leave files.c unchanged.
 */

#include <stdio.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"
#include "files_vita.h"

static char vita_root[ __MAX_PATH ] = "ux0:/data/bennugd64/";

static const char * vita_fallbacks[] = {
    "ux0:/data/bennugd64/",
    "app0:/",
    NULL
};

static int has_device( const char * path )
{
    return path && strchr( path, ':' ) != NULL;
}

static const char * strip_rel( const char * filename )
{
    if ( filename[0] == '.' && filename[1] == '/' )
        filename += 2;
    while ( filename[0] == '/' )
        filename++;
    return filename;
}

void file_vita_bind_root( const char * dcb_path )
{
    char * slash;

    snprintf( vita_root, sizeof( vita_root ), "app0:/" );

    if ( !dcb_path || !dcb_path[0] )
        return;

    if ( !has_device( dcb_path ) && !strchr( dcb_path, '/' ) )
        return;

    snprintf( vita_root, sizeof( vita_root ), "%s", dcb_path );
    slash = strrchr( vita_root, '/' );
    if ( !slash )
    {
        snprintf( vita_root, sizeof( vita_root ), "app0:/" );
        return;
    }
    slash[1] = '\0';
}

const char * file_vita_root( void )
{
    return vita_root;
}

int file_native_try_gzip( const char * filename )
{
    ( void ) filename;
    return 1;
}

FILE * file_native_fopen( const char * filename, const char * mode )
{
    char path[ __MAX_PATH ];
    FILE * fp;
    int i;

    if ( !filename || !filename[0] || !mode )
        return NULL;

    if ( has_device( filename ) )
        return fopen( filename, mode );

    filename = strip_rel( filename );

    snprintf( path, sizeof( path ), "%s%s", vita_root, filename );
    fp = fopen( path, mode );
    if ( fp )
        return fp;

    for ( i = 0 ; vita_fallbacks[i] ; i++ )
    {
        if ( strcmp( vita_fallbacks[i], vita_root ) == 0 )
            continue;
        snprintf( path, sizeof( path ), "%s%s", vita_fallbacks[i], filename );
        fp = fopen( path, mode );
        if ( fp )
            return fp;
    }

    return fopen( filename, mode );
}

int file_native_move( const char * source_file, const char * target_file )
{
    char src[ __MAX_PATH ], dst[ __MAX_PATH ];

    if ( !source_file || !target_file )
        return -1;

    if ( has_device( source_file ) )
        snprintf( src, sizeof( src ), "%s", source_file );
    else
        snprintf( src, sizeof( src ), "%s%s", vita_root, strip_rel( source_file ) );

    if ( has_device( target_file ) )
        snprintf( dst, sizeof( dst ), "%s", target_file );
    else
        snprintf( dst, sizeof( dst ), "%s%s", vita_root, strip_rel( target_file ) );

    return rename( src, dst );
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
