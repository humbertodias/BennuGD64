/*
 * PlayStation 3 file I/O. Linked instead of files_native.c.
 *
 * Games (SoRR) open relative names such as palettes/enemies/galsia.pal.
 * PSL1GHT cwd after chdir("/dev_usb000/...") is often still the ELF
 * directory (USRDIR / app_home), so a bare fopen misses
 * /dev_usb000/bennugd64/palettes/enemies/galsia.pal. Prefix with the DCB
 * directory here; leave files.c unchanged.
 *
 * Absolute paths are POSIX (/dev_hdd0/..., /dev_usb000/...), not device:.
 */

#include <stdio.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"
#include "files_ps3.h"

#ifndef BENNUGD_PS3_TITLE_ID
#define BENNUGD_PS3_TITLE_ID "BGD300001"
#endif

static char ps3_root[ __MAX_PATH ] = "/dev_usb000/bennugd64/";

static const char * ps3_fallbacks[] = {
    "/dev_usb000/bennugd64/",
    "/dev_hdd0/tmp/bennugd64/",
    "/dev_hdd0/game/" BENNUGD_PS3_TITLE_ID "/USRDIR/",
    "/app_home/",
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

void file_ps3_bind_root( const char * dcb_path )
{
    char * slash;

    snprintf( ps3_root, sizeof( ps3_root ),
              "/dev_hdd0/game/%s/USRDIR/", BENNUGD_PS3_TITLE_ID );

    if ( !dcb_path || !dcb_path[0] )
        return;

    if ( !is_abs( dcb_path ) && !strchr( dcb_path, '/' ) )
        return;

    snprintf( ps3_root, sizeof( ps3_root ), "%s", dcb_path );
    slash = strrchr( ps3_root, '/' );
    if ( !slash )
    {
        snprintf( ps3_root, sizeof( ps3_root ),
                  "/dev_hdd0/game/%s/USRDIR/", BENNUGD_PS3_TITLE_ID );
        return;
    }
    slash[1] = '\0';
}

const char * file_ps3_root( void )
{
    return ps3_root;
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

    snprintf( path, sizeof( path ), "%s%s", ps3_root, filename );
    fp = fopen( path, mode );
    if ( fp )
        return fp;

    for ( i = 0 ; ps3_fallbacks[i] ; i++ )
    {
        if ( strcmp( ps3_fallbacks[i], ps3_root ) == 0 )
            continue;
        snprintf( path, sizeof( path ), "%s%s", ps3_fallbacks[i], filename );
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
        snprintf( src, sizeof( src ), "%s%s", ps3_root, strip_rel( source_file ) );

    if ( is_abs( target_file ) )
        snprintf( dst, sizeof( dst ), "%s", target_file );
    else
        snprintf( dst, sizeof( dst ), "%s%s", ps3_root, strip_rel( target_file ) );

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
