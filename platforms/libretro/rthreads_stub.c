/* libogc already defines pthread_t. libretro-common's gx_pthread.h
 * conflicts with that, so Wii (and other GEKKO) cores skip rthreads.c. */

#include "rthreads/rthreads.h"
#include <stdlib.h>

struct slock
{
  int unused;
};

slock_t *
slock_new (void)
{
  return (slock_t *) calloc (1, sizeof (slock_t));
}

void
slock_free (slock_t *lock)
{
  free (lock);
}

void
slock_lock (slock_t *lock)
{
  (void) lock;
}

void
slock_unlock (slock_t *lock)
{
  (void) lock;
}
