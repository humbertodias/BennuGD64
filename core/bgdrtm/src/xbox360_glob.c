/*
 * Minimal glob(3) over libXenon's devoptab directory iterator.
 * libfat implements directory callbacks but libXenon's newlib intentionally
 * omits dirent.h and the POSIX glob entry points.
 */

#include <errno.h>
#include <glob.h>
#include <reent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iosupport.h>
#include <sys/stat.h>

#ifndef GLOB_NOMATCH
#define GLOB_NOMATCH (-3)
#endif

static bool xbox360_match( const char * pattern, const char * value )
{
    while ( *pattern )
    {
        if ( *pattern == '*' )
        {
            while ( *pattern == '*' ) pattern++;
            if ( !*pattern ) return true;
            while ( *value )
                if ( xbox360_match( pattern, value++ ) ) return true;
            return false;
        }
        if ( *pattern == '?' )
        {
            if ( !*value ) return false;
            pattern++;
            value++;
            continue;
        }
        if ( *pattern == '[' )
        {
            bool found = false;
            pattern++;
            while ( *pattern && *pattern != ']' )
                found |= ( *pattern++ == *value );
            if ( *pattern == ']' ) pattern++;
            if ( !found || !*value ) return false;
            value++;
            continue;
        }
        if ( *pattern++ != *value++ ) return false;
    }
    return *value == '\0';
}

int glob( const char * pattern, int flags,
          int ( * errfunc )( const char *, int ), glob_t * result )
{
    const devoptab_t * device;
    DIR_ITER * iterator;
    struct stat st;
    char filename[256];
    char * directory;
    const char * leaf;
    const char * slash;
    size_t capacity = 8;
    int device_id;
    ( void )flags;
    ( void )errfunc;

    if ( !pattern || !result )
    {
        errno = EINVAL;
        return GLOB_NOSPACE;
    }

    memset( result, 0, sizeof( *result ) );
    slash = strrchr( pattern, '/' );
    if ( slash )
    {
        size_t length = ( size_t )( slash - pattern );
        directory = malloc( length + 1 );
        if ( !directory ) return GLOB_NOSPACE;
        memcpy( directory, pattern, length );
        directory[ length ] = '\0';
        leaf = slash + 1;
    }
    else
    {
        directory = strdup( "." );
        leaf = pattern;
    }
    if ( !directory ) return GLOB_NOSPACE;
    if ( !*directory )
    {
        free( directory );
        directory = strdup( "/" );
        if ( !directory ) return GLOB_NOSPACE;
    }

    device_id = FindDevice( directory );
    if ( device_id < 0 )
    {
        free( directory );
        return GLOB_NOMATCH;
    }
    device = devoptab_list[ device_id ];
    if ( !device || !device->diropen_r || !device->dirnext_r )
    {
        free( directory );
        return GLOB_NOMATCH;
    }

    iterator = calloc( 1, sizeof( *iterator ) + ( size_t )device->dirStateSize );
    if ( !iterator )
    {
        free( directory );
        return GLOB_NOSPACE;
    }
    iterator->device = device_id;
    iterator->dirStruct = iterator + 1;
    if ( !device->diropen_r( _REENT, iterator, directory ) )
    {
        free( iterator );
        free( directory );
        return GLOB_NOMATCH;
    }

    result->gl_pathv = calloc( capacity, sizeof( char * ) );
    if ( !result->gl_pathv )
    {
        device->dirclose_r( _REENT, iterator );
        free( iterator );
        free( directory );
        return GLOB_NOSPACE;
    }

    while ( device->dirnext_r( _REENT, iterator, filename, &st ) == 0 )
    {
        char * path;
        size_t length;
        if ( !xbox360_match( leaf, filename ) ) continue;
        if ( result->gl_pathc + 1 >= capacity )
        {
            char ** expanded;
            capacity *= 2;
            expanded = realloc( result->gl_pathv, capacity * sizeof( char * ) );
            if ( !expanded ) break;
            result->gl_pathv = expanded;
        }
        length = strlen( directory ) + strlen( filename ) + 2;
        path = malloc( length );
        if ( !path ) break;
        snprintf( path, length, "%s%s%s", directory,
                  directory[ strlen( directory ) - 1 ] == '/' ? "" : "/",
                  filename );
        result->gl_pathv[ result->gl_pathc++ ] = path;
        result->gl_pathv[ result->gl_pathc ] = NULL;
    }

    if ( device->dirclose_r ) device->dirclose_r( _REENT, iterator );
    free( iterator );
    free( directory );
    return result->gl_pathc ? 0 : GLOB_NOMATCH;
}

void globfree( glob_t * result )
{
    size_t i;
    if ( !result ) return;
    for ( i = 0; i < result->gl_pathc; i++ ) free( result->gl_pathv[i] );
    free( result->gl_pathv );
    memset( result, 0, sizeof( *result ) );
}
