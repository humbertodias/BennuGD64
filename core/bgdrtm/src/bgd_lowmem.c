/*
 * Language-visible buffers (globals/locals/privates/publics) must live in the
 * low 32-bit address space on LP64: Bennu POINTER values are still 4 bytes.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) && defined(__x86_64__) && !defined(TARGET_PS4)
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "bgd_lowmem.h"

#define BGD_LOW_MAGIC_MMAP   0x324D4C42u /* 'BLM2' */
#define BGD_LOW_MAGIC_HEAP   0x32484C42u /* 'BLH2' */

typedef struct
{
    uint32_t magic;
    uint32_t total;
} bgd_low_hdr;

static void * bgd_low_alloc( size_t size, int zeroed )
{
    size_t total;
    bgd_low_hdr * hdr;
    void * map = NULL;

    if ( size > ( size_t )0x7fffffff - sizeof( bgd_low_hdr ) ) return NULL;

    total = sizeof( bgd_low_hdr ) + size;

#if defined(__linux__) && defined(__x86_64__) && !defined(TARGET_PS4)
    {
        size_t page = ( size_t ) sysconf( _SC_PAGESIZE );
        size_t mapped = ( total + page - 1 ) & ~( page - 1 );

        map = mmap( NULL, mapped, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0 );
        if ( map != MAP_FAILED )
        {
            hdr = ( bgd_low_hdr * ) map;
            hdr->magic = BGD_LOW_MAGIC_MMAP;
            hdr->total = ( uint32_t ) mapped;
            if ( zeroed ) memset( hdr + 1, 0, size );
            return hdr + 1;
        }
    }
#endif

    hdr = ( bgd_low_hdr * ) malloc( total );
    if ( !hdr ) return NULL;
    hdr->magic = BGD_LOW_MAGIC_HEAP;
    hdr->total = ( uint32_t ) total;
    if ( zeroed ) memset( hdr + 1, 0, size );
    return hdr + 1;
}

void * bgd_low_calloc( size_t size )
{
    return bgd_low_alloc( size, 1 );
}

void * bgd_low_malloc( size_t size )
{
    return bgd_low_alloc( size, 0 );
}

void bgd_low_free( void * ptr )
{
    bgd_low_hdr * hdr;

    if ( !ptr ) return;

    hdr = ( ( bgd_low_hdr * ) ptr ) - 1;
    if ( hdr->magic == BGD_LOW_MAGIC_HEAP )
    {
        hdr->magic = 0;
        free( hdr );
        return;
    }

#if defined(__linux__) && defined(__x86_64__) && !defined(TARGET_PS4)
    if ( hdr->magic == BGD_LOW_MAGIC_MMAP )
    {
        size_t total = hdr->total;
        hdr->magic = 0;
        munmap( hdr, total );
        return;
    }
#endif

    /* Not allocated by us; ignore rather than crash. */
}
