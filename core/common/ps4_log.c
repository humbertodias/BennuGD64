/* Persistent PS4 stdout/stderr and last-resort fatal-signal logging. */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ps4_log.h"

#define PS4_LOG_PATH "/data/bennugd64/bgdi.log"

static volatile sig_atomic_t handling_fatal_signal;
static int ps4_log_fd = -1;

static void ps4_log_write_bytes( const char * data, size_t size )
{
    if ( ps4_log_fd < 0 || !data || !size )
        return;
    write( ps4_log_fd, data, size );
    fsync( ps4_log_fd );
}

void ps4_log_write( const char * message )
{
    if ( !message )
        return;
    ps4_log_write_bytes( message, strlen( message ) );
    ps4_log_write_bytes( "\n", 1 );
}

static void ps4_log_fatal_signal( int signal_number )
{
    static const char prefix[] = "bgdi: fatal signal ";
    char number[ 12 ];
    unsigned int value;
    size_t length = 0;

    if ( handling_fatal_signal )
        _exit( 128 + signal_number );
    handling_fatal_signal = 1;

    value = signal_number < 0
        ? ( unsigned int )( -signal_number )
        : ( unsigned int ) signal_number;
    do
    {
        number[ length++ ] = ( char )( '0' + value % 10 );
        value /= 10;
    }
    while ( value && length < sizeof( number ) );

    ps4_log_write_bytes( prefix, sizeof( prefix ) - 1 );
    while ( length )
        ps4_log_write_bytes( &number[ --length ], 1 );
    ps4_log_write_bytes( "\n", 1 );

    signal( signal_number, SIG_DFL );
    raise( signal_number );
    _exit( 128 + signal_number );
}

static void ps4_log_install_signal_handlers( void )
{
    signal( SIGABRT, ps4_log_fatal_signal );
    signal( SIGBUS,  ps4_log_fatal_signal );
    signal( SIGFPE,  ps4_log_fatal_signal );
    signal( SIGILL,  ps4_log_fatal_signal );
    signal( SIGSEGV, ps4_log_fatal_signal );
}

int ps4_log_initialize( void )
{
    mkdir( "/data", 0777 );
    mkdir( "/data/bennugd64", 0777 );

    ps4_log_fd = open( PS4_LOG_PATH,
                       O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666 );
    if ( ps4_log_fd < 0 )
        return -1;

    ps4_log_write( "bgdi: low-level log started" );
    ps4_log_install_signal_handlers();

    freopen( PS4_LOG_PATH, "a", stderr );
    freopen( PS4_LOG_PATH, "a", stdout );
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
    ps4_log_write( "bgdi: stdio redirect returned" );
    return 0;
}
