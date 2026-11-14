/*
 * Zeebo clock. newlib nosys answers gettimeofday with zeros, so SET_FPS
 * thought every frame was instant and never skipped a late blit. The
 * emulator's virtual clock is aee_GetTimeMS (instructions, not wall time).
 */

#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define ZEEBO_MODULE_BASE 0x00010000u
/* AEEHelperFuncs::aee_GetTimeMS, from the BREW 4.0.2 helper table. */
#define ZEEBO_GETTIMEMS_SLOT 43

typedef uint32_t ( *zeebo_ms_fn )( void );

static uint32_t zeebo_ms( void )
{
    void ** table = *( void *** )( ZEEBO_MODULE_BASE - 4 );
    zeebo_ms_fn fn;

    if ( !table ) return 0;
    fn = ( zeebo_ms_fn )( uintptr_t )table[ZEEBO_GETTIMEMS_SLOT];
    if ( !fn ) return 0;
    return fn();
}

/* Enough arithmetic for the virtual clock to move about a millisecond. */
static void zeebo_burn_ms( void )
{
    volatile uint32_t n = 0;
    uint32_t i;

    for ( i = 0; i < 200000u; i++ ) n += i;
}

int clock_gettime( clockid_t clk, struct timespec * tp )
{
    uint32_t ms;

    ( void )clk;
    if ( !tp )
    {
        errno = EFAULT;
        return -1;
    }
    ms = zeebo_ms();
    tp->tv_sec = ( time_t )( ms / 1000u );
    tp->tv_nsec = ( long )( ms % 1000u ) * 1000000L;
    return 0;
}

int gettimeofday( struct timeval * tv, void * tz )
{
    uint32_t ms = zeebo_ms();

    ( void )tz;
    if ( !tv )
    {
        errno = EFAULT;
        return -1;
    }
    tv->tv_sec = ( time_t )( ms / 1000u );
    tv->tv_usec = ( suseconds_t )( ms % 1000u ) * 1000;
    return 0;
}

int nanosleep( const struct timespec * req, struct timespec * rem )
{
    uint32_t start;
    uint32_t want;

    if ( rem )
    {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    if ( !req || req->tv_sec < 0 || req->tv_nsec < 0 || req->tv_nsec >= 1000000000L )
    {
        errno = EINVAL;
        return -1;
    }
    want = ( uint32_t )req->tv_sec * 1000u + ( uint32_t )( req->tv_nsec / 1000000L );
    if ( want == 0 ) return 0;
    /* A stuck caller must not freeze the applet. One frame is enough. */
    if ( want > 40u ) want = 40u;

    start = zeebo_ms();
    zeebo_burn_ms();
    if ( zeebo_ms() == start ) return 0;

    while ( ( uint32_t )( zeebo_ms() - start ) < want )
        zeebo_burn_ms();
    return 0;
}
