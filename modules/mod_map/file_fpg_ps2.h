#ifndef __FILE_FPG_PS2_H
#define __FILE_FPG_PS2_H

#include "libgrbase.h"
#include "files.h"

int    gr_fpg_ps2_too_big( int width, int height, int bpp );
GRAPH * gr_fpg_ps2_header( int code, int width, int height, int bpp );
void   gr_fpg_ps2_skip_pixels( file * fp, int width, int height, int bpp );
void   gr_fpg_ps2_skip_rest( file * fp, int width, int height, int bpp, int ncpoints );

#endif
