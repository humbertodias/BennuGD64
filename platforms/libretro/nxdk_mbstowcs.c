/* Byte-for-wchar conversion: encoding_utf.c assumes a UTF-8 locale. */

#include "nxdk_mbstowcs.h"

size_t
mbstowcs (wchar_t *dest, const char *src, size_t n)
{
  size_t i;

  if (!src)
    return (size_t) -1;

  if (!dest)
  {
    for (i = 0; src[i]; i++)
      ;
    return i;
  }

  for (i = 0; i < n && src[i]; i++)
    dest[i] = (wchar_t) (unsigned char) src[i];
  if (i < n)
    dest[i] = L'\0';
  return i;
}

size_t
wcstombs (char *dest, const wchar_t *src, size_t n)
{
  size_t i;

  if (!src)
    return (size_t) -1;

  if (!dest)
  {
    for (i = 0; src[i]; i++)
      ;
    return i;
  }

  for (i = 0; i < n && src[i]; i++)
    dest[i] = (char) src[i];
  if (i < n)
    dest[i] = '\0';
  return i;
}
