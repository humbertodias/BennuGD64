/*
 * Minimal glob(3) for newlib (Switch libnx / Dreamcast KallistiOS): headers
 * exist, libc does not. Enough for dirs.c: GLOB_ERR | GLOB_NOSORT, fnmatch
 * on the last path component.
 */

#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef GLOB_ABEND
#ifdef GLOB_ABORTED
#define GLOB_ABEND GLOB_ABORTED
#else
#define GLOB_ABEND 2
#endif
#endif

/* newlib on some KOS/libnx trees has fnmatch.h but no fnmatch(). */
static int nx_fnmatch( const char * pat, const char * str )
{
    for ( ; ; pat++, str++ )
    {
        if ( *pat == '*' )
        {
            while ( *pat == '*' ) pat++;
            if ( !*pat ) return 0;
            for ( ; *str ; str++ )
                if ( nx_fnmatch( pat, str ) == 0 ) return 0;
            return 1;
        }
        if ( *pat == '?' )
        {
            if ( !*str ) return 1;
            continue;
        }
        if ( *pat != *str ) return 1;
        if ( !*pat ) return 0;
    }
}

int glob( const char * pattern, int flags, int ( * errfunc )( const char * epath, int eerrno ), glob_t * pglob )
{
    char * slash;
    char * dir;
    const char * pat;
    DIR * dp;
    struct dirent * ent;
    size_t cap = 8;
    (void) flags;

    if ( !pattern || !pglob )
    {
        errno = EINVAL;
        return GLOB_NOSPACE;
    }

    pglob->gl_pathc = 0;
    pglob->gl_pathv = calloc( cap, sizeof( char * ) );
    if ( !pglob->gl_pathv ) return GLOB_NOSPACE;

    slash = strrchr( pattern, '/' );
    if ( slash )
    {
        size_t dlen = ( size_t )( slash - pattern ) + 1;
        dir = malloc( dlen + 1 );
        if ( !dir )
        {
            free( pglob->gl_pathv );
            pglob->gl_pathv = NULL;
            return GLOB_NOSPACE;
        }
        memcpy( dir, pattern, dlen );
        dir[dlen] = '\0';
        pat = slash + 1;
        if ( !*pat ) pat = "*";
    }
    else
    {
        dir = strdup( "." );
        pat = pattern;
        if ( !dir )
        {
            free( pglob->gl_pathv );
            pglob->gl_pathv = NULL;
            return GLOB_NOSPACE;
        }
    }

    dp = opendir( dir );
    if ( !dp )
    {
        if ( errfunc && errfunc( dir, errno ) ) { free( dir ); globfree( pglob ); return GLOB_ABEND; }
        free( dir );
        globfree( pglob );
        return GLOB_NOMATCH;
    }

    while ( ( ent = readdir( dp ) ) )
    {
        char * full;
        size_t n;
        if ( ent->d_name[0] == '.' && ( ent->d_name[1] == '\0' || ( ent->d_name[1] == '.' && ent->d_name[2] == '\0' ) ) )
            continue;
        if ( nx_fnmatch( pat, ent->d_name ) != 0 ) continue;
        n = strlen( dir ) + strlen( ent->d_name ) + 1;
        full = malloc( n );
        if ( !full )
        {
            closedir( dp );
            free( dir );
            globfree( pglob );
            return GLOB_NOSPACE;
        }
        snprintf( full, n, "%s%s", dir, ent->d_name );
        if ( pglob->gl_pathc + 1 >= cap )
        {
            char ** nv;
            cap *= 2;
            nv = realloc( pglob->gl_pathv, cap * sizeof( char * ) );
            if ( !nv )
            {
                free( full );
                closedir( dp );
                free( dir );
                globfree( pglob );
                return GLOB_NOSPACE;
            }
            pglob->gl_pathv = nv;
        }
        pglob->gl_pathv[pglob->gl_pathc++] = full;
    }

    closedir( dp );
    free( dir );
    pglob->gl_pathv[pglob->gl_pathc] = NULL;
    return pglob->gl_pathc ? 0 : GLOB_NOMATCH;
}

void globfree( glob_t * pglob )
{
    size_t i;
    if ( !pglob || !pglob->gl_pathv ) return;
    for ( i = 0 ; i < pglob->gl_pathc ; i++ ) free( pglob->gl_pathv[i] );
    free( pglob->gl_pathv );
    pglob->gl_pathv = NULL;
    pglob->gl_pathc = 0;
}
