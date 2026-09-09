#ifndef __BGD_LOWMEM_PS4_H
#define __BGD_LOWMEM_PS4_H

#include <stddef.h>

void * bgd_lowmem_ps4_map( size_t size );
void   bgd_lowmem_ps4_unmap( void * ptr, size_t size );

#endif
