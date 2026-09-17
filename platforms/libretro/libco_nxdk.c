/* nxdk-cc is Clang without the GNU/i386 macros libco.c needs, and the
 * MSVC path includes <Windows.h> (wrong case on Linux). Use Win32
 * threads as cothreads, same idea as libco_kos.c. */

#include "libco.h"
#include <windows.h>
#include <stdlib.h>

#ifndef THD_STACK_SIZE
#define THD_STACK_SIZE (64 * 1024)
#endif

typedef struct nx_co
{
  HANDLE thd;
  HANDLE gate;
  void (*entry) (void);
  int primary;
} nx_co_t;

static nx_co_t primary;
static nx_co_t *running;
static int inited;

static void
ensure_primary (void)
{
  if (inited)
    return;
  inited = 1;
  primary.thd = GetCurrentThread ();
  primary.gate = CreateEvent (NULL, FALSE, FALSE, NULL);
  primary.primary = 1;
  running = &primary;
}

cothread_t
co_active (void)
{
  ensure_primary ();
  return running;
}

static DWORD WINAPI
thread_wrap (LPVOID arg)
{
  nx_co_t *self = (nx_co_t *) arg;
  WaitForSingleObjectEx (self->gate, INFINITE, FALSE);
  if (self->entry)
    self->entry ();
  return 0;
}

cothread_t
co_create (unsigned int size, void (*entry) (void))
{
  nx_co_t *t;
  DWORD tid = 0;

  ensure_primary ();
  t = (nx_co_t *) calloc (1, sizeof (*t));
  if (!t)
    return NULL;
  t->entry = entry;
  t->gate = CreateEvent (NULL, FALSE, FALSE, NULL);
  if (!t->gate)
    {
      free (t);
      return NULL;
    }

  if (size < THD_STACK_SIZE)
    size = THD_STACK_SIZE;
  if (size > 256 * 1024)
    size = 256 * 1024;

  t->thd = CreateThread (NULL, size, thread_wrap, t, 0, &tid);
  if (!t->thd)
    {
      CloseHandle (t->gate);
      free (t);
      return NULL;
    }
  return t;
}

void
co_delete (cothread_t handle)
{
  nx_co_t *t = (nx_co_t *) handle;
  if (!t || t->primary)
    return;
  /* nxdk has no TerminateThread. Drop our handle; the parked thread
   * keeps waiting on gate until the XBE exits. */
  if (t->thd)
    CloseHandle (t->thd);
  t->thd = NULL;
}

void
co_switch (cothread_t handle)
{
  nx_co_t *next = (nx_co_t *) handle;
  nx_co_t *prev;

  ensure_primary ();
  if (!next || next == running)
    return;
  prev = running;
  running = next;
  SetEvent (next->gate);
  WaitForSingleObjectEx (prev->gate, INFINITE, FALSE);
}
