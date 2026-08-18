#ifndef __FILES_NATIVE_H
#define __FILES_NATIVE_H

#include <stdio.h>

/*
 * Platform stdio backend for files.c.
 * Default: files_native.c. Wii: files_wii.c (same symbols).
 */

int    file_native_try_gzip( const char * filename );
FILE * file_native_fopen( const char * filename, const char * mode );

#endif
