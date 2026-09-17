/* Consoles with incomplete pthreads skip libretro-common rthreads.c. */

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
