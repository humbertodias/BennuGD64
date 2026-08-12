/*
 * Opaque 32-bit handles for native pointers stored in BennuGD variables.
 * Required on LP64 hosts where pointers do not fit in language ints.
 */

#ifndef __BGD_HANDLES_H
#define __BGD_HANDLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int   bgd_handle_put ( void * ptr );
void * bgd_handle_get ( int handle );
void  bgd_handle_free( int handle );

/* Resolve a script value that may be a handle or a raw address. */
void * bgd_ptr( intptr_t value );

#ifdef __cplusplus
}
#endif

#endif
