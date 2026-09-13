/* Minimal tlhelp32.h stub for SDL on nxdk. */
#ifndef __BENNUGD_XBOX_TLHELP32_H
#define __BENNUGD_XBOX_TLHELP32_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TH32CS_SNAPPROCESS 0x00000002

typedef struct tagPROCESSENTRY32 {
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ProcessID;
  ULONG_PTR th32DefaultHeapID;
  DWORD th32ModuleID;
  DWORD cntThreads;
  DWORD th32ParentProcessID;
  LONG pcPriClassBase;
  DWORD dwFlags;
  CHAR szExeFile[260];
} PROCESSENTRY32;

static inline HANDLE CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID)
{
  (void)dwFlags;
  (void)th32ProcessID;
  return INVALID_HANDLE_VALUE;
}

static inline BOOL Process32First(HANDLE hSnapshot, PROCESSENTRY32 *lppe)
{
  (void)hSnapshot;
  (void)lppe;
  return FALSE;
}

static inline BOOL Process32Next(HANDLE hSnapshot, PROCESSENTRY32 *lppe)
{
  (void)hSnapshot;
  (void)lppe;
  return FALSE;
}

#ifdef __cplusplus
}
#endif

#endif
