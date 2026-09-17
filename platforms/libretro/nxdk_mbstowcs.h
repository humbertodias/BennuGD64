/* nxdk pdclib has mbsrtowcs/wcsrtombs but not mbstowcs/wcstombs. */

#ifndef BENNUGD_NXDK_MBSTOWCS_H
#define BENNUGD_NXDK_MBSTOWCS_H

#include <stddef.h>
#include <wchar.h>

size_t mbstowcs (wchar_t *dest, const char *src, size_t n);
size_t wcstombs (char *dest, const wchar_t *src, size_t n);

#endif
