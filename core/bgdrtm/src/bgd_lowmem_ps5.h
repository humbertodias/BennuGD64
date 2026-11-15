#ifndef __BGD_LOWMEM_PS5_H
#define __BGD_LOWMEM_PS5_H

#include <stddef.h>

void * bgd_lowmem_ps5_map( size_t size );
void   bgd_lowmem_ps5_unmap( void * ptr, size_t size );

#endif
