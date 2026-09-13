/* Minimal SDL Windows helpers for nxdk. */
#include "SDL_internal.h"
#include "SDL_windows.h"

bool WIN_SetErrorFromHRESULT(const char *prefix, HRESULT hr)
{
    if (prefix && *prefix) {
        return SDL_SetError("%s: HRESULT 0x%08lX", prefix, (unsigned long)hr);
    }
    return SDL_SetError("HRESULT 0x%08lX", (unsigned long)hr);
}

bool WIN_SetError(const char *prefix)
{
    return WIN_SetErrorFromHRESULT(prefix, (HRESULT)GetLastError());
}

BOOL WIN_IsWine(void)
{
    return FALSE;
}
