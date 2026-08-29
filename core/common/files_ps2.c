/*
 * PlayStation 2 file I/O. Linked instead of files_native.c.
 *
 * USB (mass:) and ISO (cdfs: / cdrom0:) support fseek. Do not prefix host:
 * (PCSX2 HostFS hangs on multi-MB seeks). Skip gzopen: SoRR's 305 MB DCB as
 * gzip hangs before FRAME. Rewrite PRELOAD / BORDERLESS_SYNC so boot fits
 * 32 MiB. After the DCB is found, fopen() stays on that device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files_native.h"
#include "files_st.h"

static char fs_prefix[ 16 ];

static const char * const fs_prefixes[] = {
    "mass:/",
    "mass:",
    NULL
};

static int has_device( const char * path )
{
    return path && strchr( path, ':' ) != NULL;
}

static int is_readonly_mode( const char * mode )
{
    return mode && strchr( mode, 'r' ) && !strchr( mode, 'w' ) &&
           !strchr( mode, 'a' ) && !strchr( mode, '+' );
}

static int ends_with_ci( const char * s, const char * suf )
{
    size_t n, m, i;
    unsigned char a, b;

    if ( !s || !suf )
        return 0;
    n = strlen( s );
    m = strlen( suf );
    if ( n < m )
        return 0;
    for ( i = 0 ; i < m ; i++ )
    {
        a = ( unsigned char ) s[ n - m + i ];
        b = ( unsigned char ) suf[ i ];
        if ( a >= 'A' && a <= 'Z' ) a = ( unsigned char )( a + 32 );
        if ( b >= 'A' && b <= 'Z' ) b = ( unsigned char )( b + 32 );
        if ( a != b )
            return 0;
    }
    return 1;
}

static int is_system_txt( const char * name )
{
    const char * b;

    if ( !name || !ends_with_ci( name, "system.txt" ) )
        return 0;
    b = strrchr( name, '/' );
    if ( !b )
        b = strrchr( name, '\\' );
    b = b ? b + 1 : name;
    return ends_with_ci( b, "system.txt" );
}

static const char * strip_rel( const char * name )
{
    while ( name[0] == '.' && name[1] == '/' )
        name += 2;
    if ( name[0] == '/' )
        name++;
    return name;
}

static int starts_ci( const char * s, const char * pfx )
{
    size_t i;
    unsigned char a, b;

    if ( !s || !pfx )
        return 0;
    for ( i = 0 ; pfx[ i ] ; i++ )
    {
        a = ( unsigned char ) s[ i ];
        b = ( unsigned char ) pfx[ i ];
        if ( !a )
            return 0;
        if ( a >= 'A' && a <= 'Z' ) a = ( unsigned char )( a + 32 );
        if ( b >= 'A' && b <= 'Z' ) b = ( unsigned char )( b + 32 );
        if ( a != b )
            return 0;
    }
    return 1;
}

static FILE * fopen_writable( const char * name, const char * mode )
{
    char path[ 512 ];
    FILE * fp;
    int disc;

    disc = starts_ci( fs_prefix, "cdrom0:" ) || starts_ci( fs_prefix, "cdfs:" );
    if ( starts_ci( fs_prefix, "mass:" ) )
    {
        snprintf( path, sizeof( path ), "mass:/%s", name );
        fp = fopen( path, mode );
        if ( fp )
            return fp;
        snprintf( path, sizeof( path ), "mc0:/%s", name );
        return fopen( path, mode );
    }
    snprintf( path, sizeof( path ), "mc0:/%s", name );
    fp = fopen( path, mode );
    if ( fp || disc )
        return fp;
    snprintf( path, sizeof( path ), "mass:/%s", name );
    return fopen( path, mode );
}

void file_ps2_bind_root( const char * dcb_path )
{
    if ( !dcb_path || !dcb_path[0] )
        return;
    if ( starts_ci( dcb_path, "cdrom0:" ) )
        snprintf( fs_prefix, sizeof( fs_prefix ), "cdrom0:\\" );
    else if ( starts_ci( dcb_path, "cdfs:" ) )
        snprintf( fs_prefix, sizeof( fs_prefix ), "cdfs:/" );
    else if ( starts_ci( dcb_path, "mass:" ) )
        snprintf( fs_prefix, sizeof( fs_prefix ), "mass:/" );
}

static void upper_slash( char * dst, size_t cap, const char * src, char slash )
{
    size_t o = 0;

    while ( src[0] == '/' || src[0] == '\\' )
        src++;
    for ( ; src[0] && o + 1 < cap ; src++ )
    {
        char c = src[0];
        if ( c == '/' || c == '\\' )
            c = slash;
        else if ( c >= 'a' && c <= 'z' )
            c = ( char )( c - 32 );
        dst[ o++ ] = c;
    }
    dst[ o ] = 0;
}

static FILE * fopen_cdfs( const char * name, const char * mode )
{
    char body[ 480 ];
    char path[ 512 ];
    const char * n = name;

    if ( starts_ci( n, "cdfs:" ) )
        n += 5;
    upper_slash( body, sizeof( body ), n, '/' );
    snprintf( path, sizeof( path ), "cdfs:/%s", body );
    return fopen( path, mode );
}

static FILE * fopen_cdrom0( const char * name, const char * mode )
{
    char body[ 480 ];
    char path[ 512 ];
    char * semi;
    FILE * fp;
    const char * n = name;

    if ( starts_ci( n, "cdrom0:" ) )
        n += 7;
    upper_slash( body, sizeof( body ), n, '\\' );
    semi = strchr( body, ';' );
    if ( semi )
        *semi = 0;
    if ( strchr( body, '.' ) )
    {
        snprintf( path, sizeof( path ), "cdrom0:\\%s;1", body );
        fp = fopen( path, mode );
        if ( fp )
            return fp;
    }
    snprintf( path, sizeof( path ), "cdrom0:\\%s", body );
    return fopen( path, mode );
}

static size_t patch_sorr_system( const char * in, size_t in_len, char * out, size_t out_cap )
{
    size_t i = 0, o = 0;

    while ( i < in_len && o + 16 < out_cap )
    {
        size_t line_start = i, line_end, content_end, s, L;
        int has_nl, is_preload, is_border;

        while ( i < in_len && in[i] != '\n' && in[i] != '\r' )
            i++;
        line_end = i;
        has_nl = 0;
        if ( i < in_len && in[i] == '\r' )
        {
            i++;
            has_nl = 1;
        }
        if ( i < in_len && in[i] == '\n' )
        {
            i++;
            has_nl = 1;
        }

        content_end = line_end;
        while ( content_end > line_start &&
                ( in[content_end - 1] == ' ' || in[content_end - 1] == '\t' ) )
            content_end--;
        s = line_start;
        while ( s < content_end && ( in[s] == ' ' || in[s] == '\t' ) )
            s++;
        L = content_end - s;

        is_preload = ( L == 7 && memcmp( in + s, "PRELOAD", 7 ) == 0 );
        is_border  = ( L == 15 && memcmp( in + s, "BORDERLESS_SYNC", 15 ) == 0 );
        if ( is_preload )
        {
            memcpy( out + o, "REALTIME", 8 );
            o += 8;
        }
        else if ( is_border )
        {
            memcpy( out + o, "AUTO", 4 );
            o += 4;
        }
        else
        {
            memcpy( out + o, in + line_start, line_end - line_start );
            o += line_end - line_start;
        }
        if ( has_nl && o + 2 < out_cap )
        {
            out[o++] = '\r';
            out[o++] = '\n';
        }
    }
    return o;
}

static int is_win_drive( const char * name )
{
    unsigned char a;

    if ( !name || !name[0] || name[1] != ':' )
        return 0;
    a = ( unsigned char ) name[0];
    if ( ( a >= 'A' && a <= 'Z' ) || ( a >= 'a' && a <= 'z' ) )
        return name[2] == '\0' || name[2] == '/' || name[2] == '\\';
    return 0;
}

static FILE * fopen_ps2( const char * filename, const char * mode )
{
    char path[ 512 ];
    FILE * fp;
    const char * name = filename;
    int i;

    if ( !name || !name[0] || !mode )
        return NULL;

    if ( starts_ci( name, "host:" ) )
        return NULL;

    if ( is_win_drive( name ) )
    {
        name += 2;
        while ( name[0] == '/' || name[0] == '\\' )
            name++;
        if ( !name[0] )
            return NULL;
    }

    if ( has_device( name ) )
    {
        fp = fopen( name, mode );
        if ( fp )
            return fp;
        if ( starts_ci( name, "cdrom0:" ) )
            return fopen_cdrom0( name, mode );
        if ( starts_ci( name, "cdfs:" ) )
            return fopen_cdfs( name, mode );
        return NULL;
    }

    name = strip_rel( name );

    if ( fs_prefix[0] )
    {
        if ( starts_ci( fs_prefix, "cdrom0:" ) )
            return fopen_cdrom0( name, mode );
        if ( starts_ci( fs_prefix, "cdfs:" ) )
            return fopen_cdfs( name, mode );
        snprintf( path, sizeof( path ), "%s%s", fs_prefix, name );
        return fopen( path, mode );
    }

    for ( i = 0 ; fs_prefixes[ i ] ; i++ )
    {
        snprintf( path, sizeof( path ), "%s%s", fs_prefixes[ i ], name );
        fp = fopen( path, mode );
        if ( fp )
        {
            snprintf( fs_prefix, sizeof( fs_prefix ), "%s", fs_prefixes[ i ] );
            return fp;
        }
    }
    return fopen( filename, mode );
}

int file_native_try_gzip( const char * filename )
{
    ( void ) filename;
    return 0;
}

FILE * file_native_fopen( const char * filename, const char * mode )
{
    FILE * fp;
    char * buf;
    char * patched;
    size_t n, out_n, cap = 64 * 1024;
    FILE * tmp;

    fp = fopen_ps2( filename, mode );
    if ( !fp || !is_system_txt( filename ) || !is_readonly_mode( mode ) )
        return fp;

    buf = ( char * ) malloc( cap );
    if ( !buf )
        return fp;
    n = fread( buf, 1, cap, fp );
    fclose( fp );
    if ( n == 0 )
    {
        free( buf );
        return fopen_ps2( filename, mode );
    }

    patched = ( char * ) malloc( n + 64 );
    if ( !patched )
    {
        free( buf );
        return fopen_ps2( filename, mode );
    }
    out_n = patch_sorr_system( buf, n, patched, n + 64 );
    free( buf );

    tmp = fopen_writable( "bennugd_system_rt.tmp", "wb" );
    if ( !tmp )
    {
        free( patched );
        return fopen_ps2( filename, mode );
    }
    fwrite( patched, 1, out_n, tmp );
    fclose( tmp );
    free( patched );

    return fopen_writable( "bennugd_system_rt.tmp", "rb" );
}

int file_native_move( const char * source_file, const char * target_file )
{
    ( void ) source_file;
    ( void ) target_file;
    return -1;
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
