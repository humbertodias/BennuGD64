/* Minimal SDL_windows.h for nxdk — enough for SDL thread/timer backends. */
#ifndef SDL_windows_h_
#define SDL_windows_h_

#include "SDL_internal.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef STRICT
#define STRICT 1
#endif
#undef UNICODE
#undef _UNICODE
#ifndef MINGW32_FORCEALIGN
#define MINGW32_FORCEALIGN
#endif

#include <windows.h>
#include <winextras.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool WIN_SetError(const char *prefix);
extern bool WIN_SetErrorFromHRESULT(const char *prefix, HRESULT hr);
extern BOOL WIN_IsWine(void);

static inline char *WIN_StringToUTF8W(const WCHAR *s)
{
    (void)s;
    return NULL;
}

static inline WCHAR *WIN_UTF8ToStringW(const char *s)
{
    (void)s;
    return NULL;
}

static inline char *WIN_UTF8ToString(const char *s)
{
    return s ? SDL_strdup(s) : NULL;
}

static inline char *WIN_StringToUTF8(const char *s)
{
    return s ? SDL_strdup(s) : NULL;
}

#ifndef SDL_tcslen
#define SDL_tcslen SDL_strlen
#endif

#ifdef __cplusplus
}
#endif

#endif /* SDL_windows_h_ */
