/*
 * PlayStation 4 file I/O. Linked instead of files_native.c.
 *
 * Games open relative names such as palettes/enemies/galsia.pal. Prefix with
 * the DCB directory; leave files.c unchanged. Absolute paths are POSIX
 * (/app0/..., /data/..., /mnt/usb0/...).
 */

#include <stdio.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"
#include "files_ps4.h"

static char ps4_root[ __MAX_PATH ] = "/mnt/usb0/bennugd64/";

static const char * ps4_fallbacks[] = {
    "/mnt/usb0/bennugd64/",
    "/data/bennugd64/",
    "/app0/",
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

void file_ps4_bind_root( const char * dcb_path )
{
    char * slash;

    snprintf( ps4_root, sizeof( ps4_root ), "/app0/" );

    if ( !dcb_path || !dcb_path[0] )
        return;

    if ( !is_abs( dcb_path ) && !strchr( dcb_path, '/' ) )
        return;

    snprintf( ps4_root, sizeof( ps4_root ), "%s", dcb_path );
    slash = strrchr( ps4_root, '/' );
    if ( !slash )
    {
        snprintf( ps4_root, sizeof( ps4_root ), "/app0/" );
        return;
    }
    slash[1] = '\0';
}

const char * file_ps4_root( void )
{
    return ps4_root;
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

    snprintf( path, sizeof( path ), "%s%s", ps4_root, filename );
    fp = fopen( path, mode );
    if ( fp )
        return fp;

    for ( i = 0 ; ps4_fallbacks[i] ; i++ )
    {
        if ( strcmp( ps4_fallbacks[i], ps4_root ) == 0 )
            continue;
        snprintf( path, sizeof( path ), "%s%s", ps4_fallbacks[i], filename );
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
        snprintf( src, sizeof( src ), "%s%s", ps4_root, strip_rel( source_file ) );

    if ( is_abs( target_file ) )
        snprintf( dst, sizeof( dst ), "%s", target_file );
    else
        snprintf( dst, sizeof( dst ), "%s%s", ps4_root, strip_rel( target_file ) );

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
