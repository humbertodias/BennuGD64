/*
 * Native-title heap. SceLibcInternal's malloc stays inside a small flexible
 * heap (about 16 MiB). SorR loads the DCB, the startup FPGs and then the
 * WAV set; data/sev24.wav is where that heap runs out.
 *
 * This replaces malloc for the title with one direct-memory arena kept in
 * the low 4 GiB, so pointers still fit in a Bennu POINTER. Pointers that
 * belong to libc (strdup inside SceLibcInternal, FILE buffers) still go
 * through the real free.
 */

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef PS5_HEAP_TEST
#include "ps5_sce.h"
#endif

#define PS5_HEAP_MAGIC     0x31485053ull /* 'SPH1' little-endian */
#define PS5_HEAP_ALLOCATED 1ull
#define PS5_HEAP_ALIGN     32u
#define PS5_HEAP_MIN_SPAN  64u
#define PS5_MAP_FIXED      0x10
#define PS5_MEM_PROT       0x03 /* CPU read/write. GPU bits belong on the scanout buffers. */
/* Type 3 is write-combining (the framebuffer). CPU reads skip the cache,
 * which is why the game crawled once every allocation moved here.
 * Type 1 is write-back; type 0 is the same mode on the older numbering. */
#define PS5_MEM_TYPE_CACHED 1
#define PS5_MEM_TYPE_CACHED_LEGACY 0

typedef struct Ps5Hdr
{
    uint64_t size;
    uint64_t prev_size;
    uint64_t reserved;
    uint64_t magic;
} Ps5Hdr;

typedef struct Ps5Free
{
    Ps5Hdr   hdr;
    struct Ps5Free * next;
    struct Ps5Free * prev;
} Ps5Free;

static uint8_t * heap_base;
static size_t    heap_bytes;
static Ps5Free * heap_free;
static atomic_flag heap_lock = ATOMIC_FLAG_INIT;
static int       heap_ready;

static size_t heap_span( const Ps5Hdr * hdr )
{
    return ( size_t )( hdr->size & ~PS5_HEAP_ALLOCATED );
}

static int heap_allocated( const Ps5Hdr * hdr )
{
    return ( hdr->size & PS5_HEAP_ALLOCATED ) != 0;
}

static Ps5Hdr * heap_next( Ps5Hdr * hdr )
{
    return ( Ps5Hdr * )( ( uint8_t * ) hdr + heap_span( hdr ) );
}

static int heap_inside( const void * ptr )
{
    const uint8_t * p = ( const uint8_t * ) ptr;

    return heap_base && p >= heap_base && p < heap_base + heap_bytes;
}

static Ps5Hdr * heap_header( void * ptr )
{
    Ps5Hdr * hdr;

    if ( !heap_inside( ptr ) )
        return NULL;
    hdr = ( Ps5Hdr * ) ptr - 1;
    if ( !heap_inside( hdr ) || hdr->magic != PS5_HEAP_MAGIC )
        return NULL;
    return hdr;
}

static void heap_set_size( Ps5Hdr * hdr, size_t span, int allocated )
{
    hdr->size = ( uint64_t ) span | ( allocated ? PS5_HEAP_ALLOCATED : 0 );
    hdr->magic = PS5_HEAP_MAGIC;
    heap_next( hdr )->prev_size = ( uint64_t ) span;
}

static void heap_unlink( Ps5Free * block )
{
    if ( block->prev )
        block->prev->next = block->next;
    else
        heap_free = block->next;
    if ( block->next )
        block->next->prev = block->prev;
}

static void heap_link( Ps5Free * block )
{
    block->prev = NULL;
    block->next = heap_free;
    if ( heap_free )
        heap_free->prev = block;
    heap_free = block;
}

static size_t heap_need( size_t size )
{
    size_t span;

    if ( size < sizeof( Ps5Free ) - sizeof( Ps5Hdr ) )
        size = sizeof( Ps5Free ) - sizeof( Ps5Hdr );
    span = ( size + sizeof( Ps5Hdr ) + PS5_HEAP_ALIGN - 1 ) & ~( size_t )( PS5_HEAP_ALIGN - 1 );
    if ( span < PS5_HEAP_MIN_SPAN )
        span = PS5_HEAP_MIN_SPAN;
    return span;
}

static void * heap_malloc_unlocked( size_t size )
{
    size_t need;
    Ps5Free * block;

    if ( !heap_base || !size )
        size = 1;
    need = heap_need( size );

    for ( block = heap_free; block; block = block->next )
    {
        size_t span = heap_span( &block->hdr );
        Ps5Hdr * hdr = &block->hdr;

        if ( span < need )
            continue;

        heap_unlink( block );
        if ( span - need >= PS5_HEAP_MIN_SPAN )
        {
            Ps5Free * rest = ( Ps5Free * )( ( uint8_t * ) hdr + need );

            heap_set_size( hdr, need, 1 );
            rest->hdr.prev_size = need;
            rest->hdr.reserved = 0;
            heap_set_size( &rest->hdr, span - need, 0 );
            heap_link( rest );
        }
        else
        {
            heap_set_size( hdr, span, 1 );
        }
        hdr->reserved = 0;
        return hdr + 1;
    }
    return NULL;
}

static void heap_free_unlocked( void * ptr )
{
    Ps5Hdr * hdr = heap_header( ptr );
    Ps5Hdr * next;
    size_t span;

    if ( !hdr || !heap_allocated( hdr ) )
        return;

    span = heap_span( hdr );
    next = heap_next( hdr );
    if ( heap_inside( next ) && next->magic == PS5_HEAP_MAGIC && !heap_allocated( next ) && heap_span( next ) )
    {
        heap_unlink( ( Ps5Free * ) next );
        span += heap_span( next );
    }

    if ( hdr->prev_size )
    {
        Ps5Hdr * prev = ( Ps5Hdr * )( ( uint8_t * ) hdr - ( size_t ) hdr->prev_size );

        if ( heap_inside( prev ) && prev->magic == PS5_HEAP_MAGIC && !heap_allocated( prev ) )
        {
            heap_unlink( ( Ps5Free * ) prev );
            span += heap_span( prev );
            hdr = prev;
        }
    }

    hdr->reserved = 0;
    heap_set_size( hdr, span, 0 );
    heap_link( ( Ps5Free * ) hdr );
}

static void * heap_realloc_unlocked( void * ptr, size_t size )
{
    Ps5Hdr * hdr;
    Ps5Hdr * next;
    size_t need;
    size_t span;
    size_t old_payload;
    void * fresh;

    if ( !ptr )
        return heap_malloc_unlocked( size );
    hdr = heap_header( ptr );
    if ( !hdr )
        return NULL;
    if ( !size )
    {
        heap_free_unlocked( ptr );
        return NULL;
    }

    need = heap_need( size );
    span = heap_span( hdr );
    next = heap_next( hdr );
    if ( span < need && heap_inside( next ) && next->magic == PS5_HEAP_MAGIC &&
         !heap_allocated( next ) && heap_span( next ) && span + heap_span( next ) >= need )
    {
        heap_unlink( ( Ps5Free * ) next );
        span += heap_span( next );
        heap_set_size( hdr, span, 1 );
    }

    if ( heap_span( hdr ) >= need )
    {
        span = heap_span( hdr );
        if ( span - need >= PS5_HEAP_MIN_SPAN )
        {
            Ps5Free * rest = ( Ps5Free * )( ( uint8_t * ) hdr + need );

            heap_set_size( hdr, need, 1 );
            rest->hdr.prev_size = need;
            rest->hdr.reserved = 0;
            heap_set_size( &rest->hdr, span - need, 0 );
            heap_link( rest );
        }
        return ptr;
    }

    old_payload = heap_span( hdr ) - sizeof( Ps5Hdr );
    fresh = heap_malloc_unlocked( size );
    if ( !fresh )
        return NULL;
    memcpy( fresh, ptr, old_payload < size ? old_payload : size );
    heap_free_unlocked( ptr );
    return fresh;
}

static int heap_init_buffer( void * base, size_t bytes )
{
    size_t usable;
    Ps5Hdr * first;
    Ps5Hdr * end;

    if ( !base || bytes < sizeof( Ps5Hdr ) * 4 )
        return 0;

    usable = ( bytes - sizeof( Ps5Hdr ) ) & ~( size_t )( PS5_HEAP_ALIGN - 1 );
    if ( usable < sizeof( Ps5Free ) )
        return 0;

    memset( base, 0, bytes );
    heap_base = ( uint8_t * ) base;
    heap_bytes = bytes;
    heap_free = NULL;

    first = ( Ps5Hdr * ) base;
    first->prev_size = 0;
    first->reserved = 0;
    end = ( Ps5Hdr * )( heap_base + usable );
    end->size = PS5_HEAP_ALLOCATED;
    end->magic = PS5_HEAP_MAGIC;
    end->reserved = 0;
    heap_set_size( first, usable, 0 );
    heap_link( ( Ps5Free * ) first );
    return 1;
}

#ifndef PS5_HEAP_TEST

static void heap_lock_acquire( void )
{
    while ( atomic_flag_test_and_set_explicit( &heap_lock, memory_order_acquire ) )
        ;
}

static void heap_lock_release( void )
{
    atomic_flag_clear_explicit( &heap_lock, memory_order_release );
}

static void heap_log_size( size_t bytes )
{
    char message[ 48 ] = "bgdi: direct heap ";
    char number[ 8 ];
    size_t mib = bytes / ( 1024u * 1024u );
    size_t at = 0;
    size_t i = 0;

    do
    {
        number[ at++ ] = ( char )( '0' + ( mib % 10 ) );
        mib /= 10;
    }
    while ( mib && at < sizeof( number ) );

    while ( message[ i ] )
        i++;
    while ( at && i + 1 < sizeof( message ) )
        message[ i++ ] = number[ --at ];
    if ( i + 4 < sizeof( message ) )
    {
        message[ i++ ] = ' ';
        message[ i++ ] = 'M';
        message[ i++ ] = 'i';
        message[ i++ ] = 'B';
    }
    message[ i ] = '\0';
    sceKernelDebugOutText( 0, message );
    sceKernelDebugOutText( 0, "\n" );
}

static int heap_map( void * hint, size_t bytes, int fixed, int type )
{
    intptr_t phys = 0;
    void * addr = hint;
    int result;

    result = sceKernelAllocateMainDirectMemory( bytes, 0x200000, type, &phys );
    if ( result < 0 )
        return 0;
    result = sceKernelMapDirectMemory( &addr, bytes, PS5_MEM_PROT, fixed ? PS5_MAP_FIXED : 0,
                                       phys, 0x200000 );
    if ( result < 0 || !addr || ( uintptr_t ) addr > UINT32_MAX - bytes )
    {
        if ( result >= 0 && addr )
            sceKernelMunmap( addr, bytes );
        sceKernelReleaseDirectMemory( phys, bytes );
        return 0;
    }
    if ( !heap_init_buffer( addr, bytes ) )
    {
        sceKernelMunmap( addr, bytes );
        sceKernelReleaseDirectMemory( phys, bytes );
        return 0;
    }
    heap_log_size( bytes );
    return 1;
}

static int heap_create( void )
{
    static const size_t sizes[] = {
        1536ull * 1024ull * 1024ull,
        1024ull * 1024ull * 1024ull,
        768ull * 1024ull * 1024ull,
        512ull * 1024ull * 1024ull,
        256ull * 1024ull * 1024ull,
        128ull * 1024ull * 1024ull
    };
    static const uintptr_t hints[] = { 0x80000000ull, 0x40000000ull, 0x20000000ull };
    static const int types[] = { PS5_MEM_TYPE_CACHED, PS5_MEM_TYPE_CACHED_LEGACY };
    size_t s, h, t;

    for ( t = 0; t < sizeof( types ) / sizeof( types[ 0 ] ); t++ )
    {
        for ( s = 0; s < sizeof( sizes ) / sizeof( sizes[ 0 ] ); s++ )
        {
            for ( h = 0; h < sizeof( hints ) / sizeof( hints[ 0 ] ); h++ )
            {
                if ( ( uintptr_t ) sizes[ s ] > UINT32_MAX - hints[ h ] )
                    continue;
                if ( heap_map( ( void * ) hints[ h ], sizes[ s ], 1, types[ t ] ) ||
                     heap_map( ( void * ) hints[ h ], sizes[ s ], 0, types[ t ] ) )
                    return 1;
            }
        }
    }
    return 0;
}

static int heap_ensure( void )
{
    static atomic_flag init_lock = ATOMIC_FLAG_INIT;

    if ( heap_ready )
        return heap_ready > 0;

    while ( atomic_flag_test_and_set_explicit( &init_lock, memory_order_acquire ) )
        ;
    if ( !heap_ready )
        heap_ready = heap_create() ? 1 : -1;
    atomic_flag_clear_explicit( &init_lock, memory_order_release );
    return heap_ready > 0;
}

void * __real_malloc( size_t size );
void __real_free( void * ptr );
void * __real_realloc( void * ptr, size_t size );
void * __real_calloc( size_t count, size_t size );

void * __wrap_malloc( size_t size )
{
    void * ptr;

    if ( !heap_ensure() )
        return __real_malloc( size ? size : 1 );

    heap_lock_acquire();
    ptr = heap_malloc_unlocked( size );
    heap_lock_release();
    if ( ptr )
        return ptr;

    {
        static int exhausted;

        if ( !exhausted )
        {
            exhausted = 1;
            sceKernelDebugOutText( 0, "bgdi: direct heap exhausted\n" );
        }
    }
    return __real_malloc( size ? size : 1 );
}

void __wrap_free( void * ptr )
{
    void * raw;

    if ( !ptr )
        return;
    if ( !heap_inside( ptr ) )
    {
        __real_free( ptr );
        return;
    }

    /* posix_memalign stores the real block just before an over-aligned pointer. */
    if ( !heap_header( ptr ) )
    {
        raw = ( ( void ** ) ptr )[ -1 ];
        if ( heap_header( raw ) )
            ptr = raw;
        else
            return;
    }

    heap_lock_acquire();
    heap_free_unlocked( ptr );
    heap_lock_release();
}

void * __wrap_realloc( void * ptr, size_t size )
{
    void * fresh;

    if ( !heap_ensure() )
        return __real_realloc( ptr, size );
    if ( ptr && !heap_inside( ptr ) )
        return __real_realloc( ptr, size );
    if ( ptr && !heap_header( ptr ) )
    {
        void * raw = ( ( void ** ) ptr )[ -1 ];
        size_t old_payload;
        uintptr_t aligned;

        if ( !heap_header( raw ) )
            return NULL;
        if ( !size )
        {
            __wrap_free( ptr );
            return NULL;
        }
        old_payload = heap_span( heap_header( raw ) ) - sizeof( Ps5Hdr );
        fresh = __wrap_malloc( size + 64 );
        if ( !fresh )
            return NULL;
        aligned = ( ( uintptr_t ) fresh + sizeof( void * ) + 63u ) & ~( uintptr_t ) 63u;
        if ( old_payload > ( size_t )( ( uint8_t * ) ptr - ( uint8_t * ) raw ) )
            old_payload -= ( size_t )( ( uint8_t * ) ptr - ( uint8_t * ) raw );
        else
            old_payload = 0;
        memcpy( ( void * ) aligned, ptr, old_payload < size ? old_payload : size );
        ( ( void ** ) aligned )[ -1 ] = fresh;
        __wrap_free( raw );
        return ( void * ) aligned;
    }

    heap_lock_acquire();
    fresh = heap_realloc_unlocked( ptr, size );
    heap_lock_release();
    return fresh;
}

void * __wrap_calloc( size_t count, size_t size )
{
    size_t bytes;
    void * ptr;

    if ( count && size > ( size_t ) -1 / count )
        return NULL;
    bytes = count * size;
    ptr = __wrap_malloc( bytes ? bytes : 1 );
    if ( ptr )
        memset( ptr, 0, bytes );
    return ptr;
}

int __wrap_posix_memalign( void ** out, size_t alignment, size_t size )
{
    void * raw;
    uintptr_t aligned;

    if ( !out || alignment < sizeof( void * ) || ( alignment & ( alignment - 1 ) ) != 0 )
        return 22;
    if ( alignment <= PS5_HEAP_ALIGN )
    {
        raw = __wrap_malloc( size ? size : 1 );
        if ( !raw )
            return 12;
        *out = raw;
        return 0;
    }

    raw = __wrap_malloc( size + alignment + sizeof( void * ) );
    if ( !raw )
        return 12;
    aligned = ( ( uintptr_t ) raw + sizeof( void * ) + alignment - 1 ) & ~( alignment - 1 );
    ( ( void ** ) aligned )[ -1 ] = raw;
    *out = ( void * ) aligned;
    return 0;
}

void * __wrap_aligned_alloc( size_t alignment, size_t size )
{
    void * ptr = NULL;

    if ( __wrap_posix_memalign( &ptr, alignment, size ) != 0 )
        return NULL;
    return ptr;
}

char * __wrap_strdup( const char * text )
{
    size_t bytes;
    char * copy;

    if ( !text )
        return NULL;
    bytes = strlen( text ) + 1;
    copy = __wrap_malloc( bytes );
    if ( copy )
        memcpy( copy, text, bytes );
    return copy;
}

#else

#include <stdio.h>
#include <stdlib.h>

int main( void )
{
    static unsigned char storage[ 2 * 1024 * 1024 ];
    void * a;
    void * b;
    void * c;
    void * d;
    unsigned char * bytes;
    size_t i;

    if ( !heap_init_buffer( storage, sizeof( storage ) ) )
        return 1;
    a = heap_malloc_unlocked( 16 );
    b = heap_malloc_unlocked( 100000 );
    c = heap_malloc_unlocked( 32 );
    if ( !a || !b || !c )
        return 2;
    memset( a, 0x11, 16 );
    memset( b, 0x22, 100000 );
    heap_free_unlocked( b );
    d = heap_malloc_unlocked( 90000 );
    if ( !d )
        return 3;
    heap_free_unlocked( a );
    heap_free_unlocked( c );
    heap_free_unlocked( d );

    a = heap_malloc_unlocked( 64 );
    b = heap_realloc_unlocked( a, 1000 );
    if ( !b )
        return 4;
    bytes = b;
    for ( i = 0; i < 64; i++ )
        bytes[ i ] = ( unsigned char ) i;
    c = heap_realloc_unlocked( b, 4000 );
    if ( !c )
        return 5;
    bytes = c;
    for ( i = 0; i < 64; i++ )
        if ( bytes[ i ] != ( unsigned char ) i )
            return 6;
    heap_free_unlocked( c );

    for ( i = 0; i < 10000; i++ )
    {
        size_t n = 16 + ( i * 37 ) % 8000;
        a = heap_malloc_unlocked( n );
        b = heap_malloc_unlocked( n / 2 + 8 );
        if ( !a || !b )
            return 7;
        memset( a, 0x5a, n );
        heap_free_unlocked( a );
        c = heap_realloc_unlocked( b, n * 2 );
        if ( !c )
            return 8;
        heap_free_unlocked( c );
    }

    a = heap_malloc_unlocked( 1024 * 1024 );
    if ( !a )
        return 9;
    heap_free_unlocked( a );
    printf( "ps5 heap self-test ok\n" );
    return 0;
}

#endif
