/* Persistent PS5 stdout/stderr and last-resort fatal-signal logging. */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ps5_log.h"

#define PS5_LOG_PATH "/data/bennugd64/bgdi.log"

#ifdef BENNUGD_PS5_NATIVE_TITLE
#include "ps5_sce.h"
#define PS5_NATIVE_LOG_PATH "/download0/bgdi.log"
#endif

static volatile sig_atomic_t handling_fatal_signal;
static int ps5_log_fd = -1;

static void ps5_log_write_bytes( const char * data, size_t size )
{
    if ( ps5_log_fd < 0 || !data || !size )
        return;
    write( ps5_log_fd, data, size );
#ifndef BENNUGD_PS5_NATIVE_TITLE
    fsync( ps5_log_fd );
#endif
}

void ps5_log_write( const char * message )
{
    if ( !message )
        return;
#ifdef BENNUGD_PS5_NATIVE_TITLE
    if ( ps5_log_fd < 0 )
    {
        sceKernelDebugOutText( 0, message );
        sceKernelDebugOutText( 0, "\n" );
        return;
    }
#endif
    ps5_log_write_bytes( message, strlen( message ) );
    ps5_log_write_bytes( "\n", 1 );
}

static void ps5_log_fatal_signal( int signal_number )
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

    ps5_log_write_bytes( prefix, sizeof( prefix ) - 1 );
    while ( length )
        ps5_log_write_bytes( &number[ --length ], 1 );
    ps5_log_write_bytes( "\n", 1 );

    signal( signal_number, SIG_DFL );
    raise( signal_number );
    _exit( 128 + signal_number );
}

static void ps5_log_install_signal_handlers( void )
{
    signal( SIGABRT, ps5_log_fatal_signal );
    signal( SIGBUS,  ps5_log_fatal_signal );
    signal( SIGFPE,  ps5_log_fatal_signal );
    signal( SIGILL,  ps5_log_fatal_signal );
    signal( SIGSEGV, ps5_log_fatal_signal );
}

int ps5_log_initialize( void )
{
    const char * path = PS5_LOG_PATH;
#ifndef BENNUGD_PS5_NATIVE_TITLE
    mkdir( "/data", 0777 );
    mkdir( "/data/bennugd64", 0777 );
#endif

    const int flags = O_WRONLY | O_CREAT | O_TRUNC | O_APPEND;
    ps5_log_fd = open( path, flags, 0666 );
#ifdef BENNUGD_PS5_NATIVE_TITLE
    if ( ps5_log_fd < 0 )
    {
        path = PS5_NATIVE_LOG_PATH;
        ps5_log_fd = open( path, flags, 0666 );
    }
#endif
    if ( ps5_log_fd < 0 )
        return -1;

    ps5_log_write( "bgdi: low-level log started" );
    ps5_log_install_signal_handlers();

#ifdef BENNUGD_PS5_NATIVE_TITLE
    FILE * log_stream = fopen( path, "a" );
    if ( log_stream )
    {
        if ( stderr )
            fclose( stderr );
        if ( stdout )
            fclose( stdout );
        stdout = stderr = log_stream;
        setvbuf( log_stream, NULL, _IOLBF, 0 );
    }
#else
    freopen( path, "a", stderr );
    freopen( path, "a", stdout );
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
#endif
    ps5_log_write( "bgdi: stdio redirect returned" );
    return 0;
}
