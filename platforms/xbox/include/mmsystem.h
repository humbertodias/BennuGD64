/* Minimal mmsystem.h stub for SDL_timer.c on nxdk. */
#ifndef __BENNUGD_XBOX_MMSYSTEM_H
#define __BENNUGD_XBOX_MMSYSTEM_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UINT MMRESULT;
#define TIMERR_NOERROR 0

static inline MMRESULT timeBeginPeriod(UINT uPeriod)
{
  (void)uPeriod;
  return TIMERR_NOERROR;
}

static inline MMRESULT timeEndPeriod(UINT uPeriod)
{
  (void)uPeriod;
  return TIMERR_NOERROR;
}

#ifdef __cplusplus
}
#endif

#endif
