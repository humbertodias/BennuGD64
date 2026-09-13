/* Extra WinAPI types/macros SDL3 expects that nxdk omits.
 * Include after <windows.h> (see platforms/xbox/sdl/SDL_windows.h).
 */
#ifndef __BENNUGD_XBOX_WINEXTRAS_H
#define __BENNUGD_XBOX_WINEXTRAS_H

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TEXT
#define TEXT(x) x
#endif

#ifndef TIMER_ALL_ACCESS
#define TIMER_ALL_ACCESS 0x1F0003
#endif

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

#ifndef LPCWCH
typedef const WCHAR *LPCWCH;
#endif
#ifndef LPCCH
typedef const CHAR *LPCCH;
#endif
#ifndef LPBOOL
typedef BOOL *LPBOOL;
#endif
#ifndef LPWSTR
typedef WCHAR *LPWSTR;
#endif
#ifndef LPWCH
typedef WCHAR *LPWCH;
#endif

#ifndef RECT
typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *LPRECT;
#endif

#ifndef HICON
typedef void *HICON;
#endif

#ifndef SEM_FAILCRITICALERRORS
#define SEM_FAILCRITICALERRORS 0x0001
#define SEM_NOOPENFILEERRORBOX 0x8000
#endif

#ifndef FILE_TYPE_DISK
#define FILE_TYPE_DISK 1
#endif
#ifndef FILE_TYPE_PIPE
#define FILE_TYPE_PIPE 3
#endif

#ifndef GENERIC_READ
#define GENERIC_READ 0x80000000
#endif
#ifndef GENERIC_WRITE
#define GENERIC_WRITE 0x40000000
#endif
#ifndef OPEN_EXISTING
#define OPEN_EXISTING 3
#endif
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS 2
#endif
#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif
#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE 0x00000002
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif

#ifndef GetModuleHandle
static inline HMODULE GetModuleHandle(LPCSTR lpModuleName)
{
  (void)lpModuleName;
  return NULL;
}
#endif

#ifndef GetEnvironmentStringsW
static inline LPWCH GetEnvironmentStringsW(void)
{
  return NULL;
}
#endif

#ifndef FreeEnvironmentStringsW
static inline BOOL FreeEnvironmentStringsW(LPWCH strings)
{
  (void)strings;
  return TRUE;
}
#endif

#ifndef GetEnvironmentVariableA
static inline DWORD GetEnvironmentVariableA(LPCSTR name, LPSTR buffer, DWORD size)
{
  (void)name;
  (void)buffer;
  (void)size;
  return 0;
}
#endif

#ifndef SetEnvironmentVariableA
static inline BOOL SetEnvironmentVariableA(LPCSTR name, LPCSTR value)
{
  (void)name;
  (void)value;
  return FALSE;
}
#endif

#ifndef SetErrorMode
static inline UINT SetErrorMode(UINT uMode)
{
  (void)uMode;
  return 0;
}
#endif

#ifndef CreateFileW
static inline HANDLE CreateFileW(LPCWSTR name, DWORD access, DWORD share,
                                 LPSECURITY_ATTRIBUTES sa, DWORD disposition,
                                 DWORD flags, HANDLE templateFile)
{
  (void)name;
  (void)access;
  (void)share;
  (void)sa;
  (void)disposition;
  (void)flags;
  (void)templateFile;
  return INVALID_HANDLE_VALUE;
}
#endif

#ifndef GetFullPathNameA
static inline DWORD GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength,
                                     LPSTR lpBuffer, LPSTR *lpFilePart)
{
  size_t n;
  if (!lpFileName || !lpBuffer || nBufferLength == 0) return 0;
  n = strlen(lpFileName);
  if (n + 1 > (size_t)nBufferLength) return (DWORD)(n + 1);
  memcpy(lpBuffer, lpFileName, n + 1);
  if (lpFilePart)
  {
    char *slash = strrchr(lpBuffer, '\\');
    char *slash2 = strrchr(lpBuffer, '/');
    if (slash2 && (!slash || slash2 > slash)) slash = slash2;
    *lpFilePart = slash ? slash + 1 : lpBuffer;
  }
  return (DWORD)n;
}
#endif

#ifndef GetFullPathName
#define GetFullPathName GetFullPathNameA
#endif

#ifndef FlushFileBuffers
static inline BOOL FlushFileBuffers(HANDLE hFile)
{
  (void)hFile;
  return FALSE;
}
#endif

#ifndef GetFileType
static inline DWORD GetFileType(HANDLE hFile)
{
  (void)hFile;
  return FILE_TYPE_DISK;
}
#endif

#ifndef GetDoubleClickTime
static inline UINT GetDoubleClickTime(void)
{
  return 500;
}
#endif

/* SetFilePointerEx / GetFileSizeEx come from nxdk libwinapi — do not stub. */

#ifndef FILE_BEGIN
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#endif

#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS 4
#endif
#ifndef CREATE_NEW
#define CREATE_NEW 1
#endif

#ifndef LPTSTR
typedef char *LPTSTR;
#endif

#ifndef GetCurrentProcess
static inline HANDLE GetCurrentProcess(void)
{
  return (HANDLE)(LONG_PTR)-1;
}
#endif

#ifndef TerminateProcess
static inline BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode)
{
  (void)hProcess;
  exit((int)uExitCode);
  return TRUE;
}
#endif

#ifndef ExitProcess
static inline void ExitProcess(UINT uExitCode)
{
  exit((int)uExitCode);
}
#endif

#ifndef ATTACH_PARENT_PROCESS
#define ATTACH_PARENT_PROCESS ((DWORD)-1)
#endif
#ifndef STD_ERROR_HANDLE
#define STD_ERROR_HANDLE ((DWORD)-12)
#endif
#ifndef ERROR_INVALID_HANDLE
#define ERROR_INVALID_HANDLE 6
#endif
#ifndef ERROR_GEN_FAILURE
#define ERROR_GEN_FAILURE 31
#endif
#ifndef ERROR_ACCESS_DENIED
#define ERROR_ACCESS_DENIED 5
#endif

#ifndef AttachConsole
static inline BOOL AttachConsole(DWORD dwProcessId)
{
  (void)dwProcessId;
  return FALSE;
}
#endif

#ifndef GetStdHandle
static inline HANDLE GetStdHandle(DWORD nStdHandle)
{
  (void)nStdHandle;
  return INVALID_HANDLE_VALUE;
}
#endif

#ifndef GetConsoleMode
static inline BOOL GetConsoleMode(HANDLE hConsoleHandle, LPDWORD lpMode)
{
  (void)hConsoleHandle;
  (void)lpMode;
  return FALSE;
}
#endif

#ifndef WriteConsole
static inline BOOL WriteConsole(HANDLE hConsoleOutput, const void *lpBuffer,
                                DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten,
                                LPVOID lpReserved)
{
  (void)hConsoleOutput;
  (void)lpBuffer;
  (void)nNumberOfCharsToWrite;
  (void)lpNumberOfCharsWritten;
  (void)lpReserved;
  return FALSE;
}
#endif

#ifndef OutputDebugString
static inline void OutputDebugString(LPCSTR lpOutputString)
{
  (void)lpOutputString;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
