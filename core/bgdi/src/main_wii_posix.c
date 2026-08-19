/*
 * POSIX symbols SDL3's cmake detects via try_compile (STATIC_LIBRARY) but
 * libogc newlib does not actually provide at link time.
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

int sigaction( int sig, const struct sigaction * act, struct sigaction * oact )
{
    ( void ) sig;
    ( void ) act;
    ( void ) oact;
    errno = ENOSYS;
    return -1;
}

int fdatasync( int fd )
{
    return fsync( fd );
}

int pthread_setname_np( pthread_t thread, const char * name )
{
    ( void ) thread;
    ( void ) name;
    return 0;
}
