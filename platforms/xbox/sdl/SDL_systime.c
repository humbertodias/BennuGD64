/* SDL time backend for nxdk. */
#include "SDL_internal.h"

#ifdef SDL_TIME_WINDOWS

#include <windows.h>
#include <winextras.h>
#include "../SDL_time_c.h"

void SDL_GetSystemTimeLocalePreferences(SDL_DateFormat *df, SDL_TimeFormat *tf)
{
    if (df) {
        *df = SDL_DATE_FORMAT_YYYYMMDD;
    }
    if (tf) {
        *tf = SDL_TIME_FORMAT_24HR;
    }
}

bool SDL_GetCurrentTime(SDL_Time *ticks)
{
    FILETIME ft;

    if (!ticks) {
        return SDL_InvalidParamError("ticks");
    }

    GetSystemTimeAsFileTime(&ft);
    *ticks = SDL_TimeFromWindows(ft.dwLowDateTime, ft.dwHighDateTime);
    return true;
}

bool SDL_TimeToDateTime(SDL_Time ticks, SDL_DateTime *dt, bool localTime)
{
    FILETIME ft;
    SYSTEMTIME st;
    Uint32 low, high;

    (void)localTime;

    if (!dt) {
        return SDL_InvalidParamError("dt");
    }

    SDL_TimeToWindows(ticks, &low, &high);
    ft.dwLowDateTime = (DWORD)low;
    ft.dwHighDateTime = (DWORD)high;

    if (!FileTimeToSystemTime(&ft, &st)) {
        return SDL_SetError("SDL_DateTime conversion failed");
    }

    dt->year = st.wYear;
    dt->month = st.wMonth;
    dt->day = st.wDay;
    dt->hour = st.wHour;
    dt->minute = st.wMinute;
    dt->second = st.wSecond;
    dt->nanosecond = (int)(ticks % SDL_NS_PER_SECOND);
    dt->day_of_week = st.wDayOfWeek;
    dt->utc_offset = 0;
    return true;
}

#endif /* SDL_TIME_WINDOWS */
