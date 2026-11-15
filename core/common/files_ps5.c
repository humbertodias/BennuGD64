/*
 * PlayStation 5 file I/O. Linked instead of files_native.c.
 *
 * Relative names are prefixed with the DCB directory. Absolute paths stay
 * as given (/data, /mnt/usb0, /user/homebrew).
 */

#include <stdio.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"
#include "files_ps5.h"

static char ps5_root[ __MAX_PATH ] = "/data/bennugd64/";

static const char * ps5_fallbacks[] = {
    "/mnt/usb0/bennugd64/",
    "/data/bennugd64/",
    "/user/homebrew/bennugd64/",
    NULL
};

static int is_abs( const char * path )
{
    return path && path[0] == '/';
}

static const char * strip_rel( const char * filename )
{
    if ( filename[0] == '.' && filename[1] == '/' )
        filename += 2;
    return filename;
}

void file_ps5_bind_root( const char * dcb_path )
{
    char * slash;

    snprintf( ps5_root, sizeof( ps5_root ), "/data/bennugd64/" );

    if ( !dcb_path || !dcb_path[0] )
        return;

    if ( !is_abs( dcb_path ) && !strchr( dcb_path, '/' ) )
        return;

    snprintf( ps5_root, sizeof( ps5_root ), "%s", dcb_path );
    slash = strrchr( ps5_root, '/' );
    if ( !slash )
    {
        snprintf( ps5_root, sizeof( ps5_root ), "/data/bennugd64/" );
        return;
    }
    slash[1] = '\0';
}

const char * file_ps5_root( void )
{
    return ps5_root;
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

    if ( is_abs( filename ) )
        return fopen( filename, mode );

    filename = strip_rel( filename );

    snprintf( path, sizeof( path ), "%s%s", ps5_root, filename );
    fp = fopen( path, mode );
    if ( fp )
        return fp;

    for ( i = 0 ; ps5_fallbacks[i] ; i++ )
    {
        if ( strcmp( ps5_fallbacks[i], ps5_root ) == 0 )
            continue;
        snprintf( path, sizeof( path ), "%s%s", ps5_fallbacks[i], filename );
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

    if ( is_abs( source_file ) )
        snprintf( src, sizeof( src ), "%s", source_file );
    else
        snprintf( src, sizeof( src ), "%s%s", ps5_root, strip_rel( source_file ) );

    if ( is_abs( target_file ) )
        snprintf( dst, sizeof( dst ), "%s", target_file );
    else
        snprintf( dst, sizeof( dst ), "%s%s", ps5_root, strip_rel( target_file ) );

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
