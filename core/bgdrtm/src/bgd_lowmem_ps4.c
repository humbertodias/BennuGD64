/* Keep language-visible PS4 buffers addressable by Bennu's 32-bit POINTER. */

#include <stdint.h>
#include <sys/mman.h>

#include "bgd_lowmem_ps4.h"

void * bgd_lowmem_ps4_map( size_t size )
{
    static uintptr_t next_hint = 0x10000000u;
    const size_t page = 16384;
    void * map = mmap( ( void * ) next_hint, size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANON, -1, 0 );

    if ( map == MAP_FAILED )
        return NULL;
    if ( ( uintptr_t ) map + size - 1 > UINT32_MAX )
    {
        munmap( map, size );
        return NULL;
    }

    next_hint = ( ( uintptr_t ) map + size + page - 1 ) & ~( page - 1 );
    return map;
}

void bgd_lowmem_ps4_unmap( void * ptr, size_t size )
{
    munmap( ptr, size );
}
