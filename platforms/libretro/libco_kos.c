/* KallistiOS has no sigsetjmp/sigaltstack. libco's sjlj backend is for
 * UNIX; use KOS threads as cothreads (same idea as libco/psp1.c). */

#include "libco.h"
#include <kos/thread.h>
#include <kos/sem.h>
#include <stdlib.h>
#include <string.h>

#ifndef THD_STACK_SIZE
#define THD_STACK_SIZE (64 * 1024)
#endif

typedef struct kos_co
{
  kthread_t *thd;
  semaphore_t gate;
  void (*entry) (void);
  int primary;
} kos_co_t;

static kos_co_t primary;
static kos_co_t *running;
static int inited;

static void
ensure_primary (void)
{
  if (inited)
    return;
  inited = 1;
  primary.thd = thd_get_current ();
  primary.primary = 1;
  sem_init (&primary.gate, 0);
  running = &primary;
}

cothread_t
co_active (void)
{
  ensure_primary ();
  return running;
}

static void *
thread_wrap (void *arg)
{
  kos_co_t *self = (kos_co_t *) arg;
  sem_wait (&self->gate);
  if (self->entry)
    self->entry ();
  return NULL;
}

cothread_t
co_create (unsigned int size, void (*entry) (void))
{
  kos_co_t *t;
  kthread_attr_t attr;

  ensure_primary ();
  t = (kos_co_t *) calloc (1, sizeof (*t));
  if (!t)
    return NULL;
  t->entry = entry;
  sem_init (&t->gate, 0);

  if (size < THD_STACK_SIZE)
    size = THD_STACK_SIZE;
  /* 16MB machine; libretro.c asks for several MB. */
  if (size > 256 * 1024)
    size = 256 * 1024;

  memset (&attr, 0, sizeof (attr));
  attr.stack_size = size;
  attr.prio = PRIO_DEFAULT;
  attr.label = "bennugd_libco";
  t->thd = thd_create_ex (&attr, thread_wrap, t);
  if (!t->thd)
    {
      sem_destroy (&t->gate);
      free (t);
      return NULL;
    }
  return t;
}

void
co_delete (cothread_t handle)
{
  kos_co_t *t = (kos_co_t *) handle;
  if (!t || t->primary)
    return;
  thd_destroy (t->thd);
  sem_destroy (&t->gate);
  free (t);
}

void
co_switch (cothread_t handle)
{
  kos_co_t *next = (kos_co_t *) handle;
  kos_co_t *prev;

  ensure_primary ();
  if (!next || next == running)
    return;
  prev = running;
  running = next;
  sem_signal (&next->gate);
  sem_wait (&prev->gate);
}
