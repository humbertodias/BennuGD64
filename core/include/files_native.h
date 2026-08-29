#ifndef __FILES_NATIVE_H
#define __FILES_NATIVE_H

#include <stdio.h>
#include <stddef.h>

#include "files_st.h"

/*
 * Platform stdio backend for files.c.
 * Default: files_native.c. Wii: files_wii.c. PS2: files_ps2.c.
 */

int    file_native_try_gzip( const char * filename );
FILE * file_native_fopen( const char * filename, const char * mode );
int    file_native_move( const char * source_file, const char * target_file );

/* Size: -1 = use SEEK_END. Seek: stdio fseek. */
int    file_native_size( file * fp );
int    file_native_seek( file * fp, int pos, int where );

/* PS2: lock later fopen()s to the device that held the DCB. */
void   file_ps2_bind_root( const char * dcb_path );

#endif
