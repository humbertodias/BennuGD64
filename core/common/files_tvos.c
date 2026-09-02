/*
 * Apple tvOS file I/O. Linked instead of files_native.c.
 *
 * Startup chdir()s to Documents (writable); the DCB and assets live in the
 * .app bundle. Relative fopen is prefixed with the DCB directory; absolute
 * paths are used as-is. Leave files.c unchanged.
 */

#include <stdio.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"
#include "files_tvos.h"

static char tvos_root[ __MAX_PATH ] = "";
static char tvos_bundle[ __MAX_PATH ] = "";
static char tvos_docs[ __MAX_PATH ] = "";

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

static int is_write_mode( const char * mode )
{
    return mode && ( strchr( mode, 'w' ) || strchr( mode, 'a' ) || strchr( mode, '+' ) );
}

static void slash_term( char * path, size_t sz )
{
    size_t n = strlen( path );

    if ( !n || path[ n - 1 ] == '/' )
        return;
    if ( n + 1 < sz )
    {
        path[ n ] = '/';
        path[ n + 1 ] = '\0';
    }
}

static void copy_root( char * dst, size_t dst_sz, const char * src )
{
    if ( !src || !src[0] )
    {
        dst[0] = '\0';
        return;
    }
    snprintf( dst, dst_sz, "%s", src );
    slash_term( dst, dst_sz );
}

void file_tvos_set_roots( const char * bundle, const char * docs )
{
    copy_root( tvos_bundle, sizeof( tvos_bundle ), bundle );
    copy_root( tvos_docs, sizeof( tvos_docs ), docs );
    if ( !tvos_root[0] && tvos_bundle[0] )
        snprintf( tvos_root, sizeof( tvos_root ), "%s", tvos_bundle );
}

void file_tvos_bind_root( const char * dcb_path )
{
    char * slash;

    if ( tvos_bundle[0] )
        snprintf( tvos_root, sizeof( tvos_root ), "%s", tvos_bundle );
    else if ( tvos_docs[0] )
        snprintf( tvos_root, sizeof( tvos_root ), "%s", tvos_docs );
    else
        tvos_root[0] = '\0';

    if ( !dcb_path || !dcb_path[0] )
        return;

    if ( !is_abs( dcb_path ) && !strchr( dcb_path, '/' ) )
        return;

    snprintf( tvos_root, sizeof( tvos_root ), "%s", dcb_path );
    slash = strrchr( tvos_root, '/' );
    if ( !slash )
    {
        if ( tvos_bundle[0] )
            snprintf( tvos_root, sizeof( tvos_root ), "%s", tvos_bundle );
        return;
    }
    slash[1] = '\0';
}

const char * file_tvos_root( void )
{
    return tvos_root;
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
    const char * fallbacks[ 4 ];
    int n = 0, i;

    if ( !filename || !filename[0] || !mode )
        return NULL;

    if ( is_abs( filename ) )
        return fopen( filename, mode );

    filename = strip_rel( filename );

    if ( is_write_mode( mode ) )
    {
        if ( tvos_docs[0] )
        {
            snprintf( path, sizeof( path ), "%s%s", tvos_docs, filename );
            return fopen( path, mode );
        }
        if ( tvos_root[0] )
        {
            snprintf( path, sizeof( path ), "%s%s", tvos_root, filename );
            return fopen( path, mode );
        }
        return fopen( filename, mode );
    }

    if ( tvos_root[0] )
        fallbacks[ n++ ] = tvos_root;
    if ( tvos_docs[0] )
        fallbacks[ n++ ] = tvos_docs;
    if ( tvos_bundle[0] )
        fallbacks[ n++ ] = tvos_bundle;
    fallbacks[ n ] = NULL;

    for ( i = 0 ; fallbacks[i] ; i++ )
    {
        if ( i > 0 && strcmp( fallbacks[i], tvos_root ) == 0 )
            continue;
        if ( i > 1 && tvos_docs[0] && strcmp( fallbacks[i], tvos_docs ) == 0 )
            continue;
        snprintf( path, sizeof( path ), "%s%s", fallbacks[i], filename );
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
    else if ( tvos_root[0] )
        snprintf( src, sizeof( src ), "%s%s", tvos_root, strip_rel( source_file ) );
    else
        snprintf( src, sizeof( src ), "%s", source_file );

    if ( is_abs( target_file ) )
        snprintf( dst, sizeof( dst ), "%s", target_file );
    else if ( tvos_docs[0] )
        snprintf( dst, sizeof( dst ), "%s%s", tvos_docs, strip_rel( target_file ) );
    else if ( tvos_root[0] )
        snprintf( dst, sizeof( dst ), "%s%s", tvos_root, strip_rel( target_file ) );
    else
        snprintf( dst, sizeof( dst ), "%s", target_file );

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
