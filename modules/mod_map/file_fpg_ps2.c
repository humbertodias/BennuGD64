/*
 * PlayStation 2 FPG parse. Compiled only into the ps2-mips build.
 *
 * file_seek on a DCB xfile returns the new offset, not 0. Treating that
 * as failure double-skipped the payload and desynced the rest of the FPG.
 */

#include "file_fpg_ps2.h"

static int row_widthb( int width, int bpp )
{
    int wb;

    if ( width < 1 || bpp < 1 )
        return 0;
    wb = width * bpp / 8;
    if ( ( wb * 8 / bpp ) < width )
        wb++;
    return wb;
}

static int on_disk_pixels( int width, int height, int bpp )
{
    switch ( bpp )
    {
        case 32:
            return width * height * 4;
        case 16:
            return width * height * 2;
        default:
            return row_widthb( width, bpp ) * height;
    }
}

static void skip_n( file * fp, int n )
{
    if ( n <= 0 )
        return;
    /* XFILE seek returns the new pos (>=0), not fseek's 0. */
    file_seek( fp, n, SEEK_CUR );
}

int gr_fpg_ps2_too_big( int width, int height, int bpp )
{
    (void) width;
    (void) height;
    (void) bpp;
    return 0;
}

GRAPH * gr_fpg_ps2_header( int code, int width, int height, int bpp )
{
    int wb, pitch;

    wb = row_widthb( width, bpp );
    pitch = wb;
    if ( pitch & 0x03 )
        pitch = ( pitch & ~3 ) + 4;
    return bitmap_new_ex( code, width, height, bpp, NULL, pitch );
}

void gr_fpg_ps2_skip_pixels( file * fp, int width, int height, int bpp )
{
    skip_n( fp, on_disk_pixels( width, height, bpp ) );
}

void gr_fpg_ps2_skip_rest( file * fp, int width, int height, int bpp, int ncpoints )
{
    if ( ncpoints < 0 )
        ncpoints = 0;
    skip_n( fp, ncpoints * 4 + on_disk_pixels( width, height, bpp ) );
}
