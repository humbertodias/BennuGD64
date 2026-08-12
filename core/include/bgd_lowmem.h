/*
 * Allocate Bennu language-visible data in the low 32-bit address range.
 * Language POINTER/INT slots are still 4 bytes; real host pointers are 8.
 */

#ifndef __BGD_LOWMEM_H
#define __BGD_LOWMEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void * bgd_low_calloc( size_t size );
void * bgd_low_malloc( size_t size );
void   bgd_low_free( void * ptr );

#ifdef __cplusplus
}
#endif

#endif
