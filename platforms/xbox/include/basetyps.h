/* Minimal basetyps.h for SDL3 + nxdk (no COM GUID stack). */
#ifndef __BENNUGD_XBOX_BASETYPS_H
#define __BENNUGD_XBOX_BASETYPS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} GUID;

typedef GUID IID;
typedef const IID *REFIID;
typedef GUID CLSID;
typedef const GUID *REFGUID;

#ifdef __cplusplus
}
#endif

#endif
