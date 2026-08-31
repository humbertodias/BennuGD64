/*
 * Compile-time capability flags for homebrew / sandboxed ports.
 * Prefer these over growing per-file lists of TARGET_* / __PSP__ tests.
 */

#ifndef __BGD_PLATFORM_H
#define __BGD_PLATFORM_H

#if defined(__EMSCRIPTEN__) || defined(TARGET_EMSCRIPTEN) \
 || defined(__SWITCH__) || defined(TARGET_SWITCH) \
 || defined(_arch_dreamcast) || defined(TARGET_DC) \
 || defined(__PSP__) || defined(TARGET_PSP) \
 || defined(__vita__) || defined(TARGET_VITA) \
 || defined(TARGET_TVOS) \
 || defined(__wii__) || defined(TARGET_WII) \
 || defined(TARGET_PS2)
#define BGD_NO_PROCESS_SPAWN 1
#endif

#if defined(__wasi__) || defined(TARGET_WASI) \
 || defined(__SWITCH__) || defined(TARGET_SWITCH) \
 || defined(_arch_dreamcast) || defined(TARGET_DC) \
 || defined(__PSP__) || defined(TARGET_PSP) \
 || defined(__vita__) || defined(TARGET_VITA) \
 || defined(TARGET_TVOS) \
 || defined(__wii__) || defined(TARGET_WII) \
 || defined(TARGET_PS2)
#define BGD_NO_DLOPEN 1
#endif

#if defined(TARGET_SWITCH) || defined(TARGET_DC) || defined(TARGET_PSP) \
 || defined(TARGET_VITA) || defined(TARGET_TVOS) || defined(TARGET_WII) || defined(TARGET_PS2) \
 || defined(__SWITCH__) || defined(_arch_dreamcast) || defined(__PSP__) \
 || defined(__vita__) || defined(__wii__)
#define BGD_NO_UTSNAME 1
#endif

#if defined(TARGET_MAC) || defined(TARGET_BEOS) || defined(TARGET_ANDROID) \
 || defined(TARGET_SWITCH) || defined(TARGET_DC) || defined(TARGET_PSP) \
 || defined(TARGET_VITA) || defined(TARGET_TVOS) || defined(TARGET_WII) || defined(TARGET_PS2)
#define BGD_GLOB_NO_PERIOD 1
#endif

#if defined(TARGET_MAC) || defined(TARGET_WII) || defined(TARGET_EMSCRIPTEN) \
 || defined(TARGET_SWITCH) || defined(TARGET_DC) || defined(TARGET_PSP) \
 || defined(TARGET_VITA) || defined(TARGET_TVOS) || defined(TARGET_PANDORA) || defined(TARGET_PS2)
#define BGD_NO_SYSINFO_MEM 1
#endif

#if defined(__EMSCRIPTEN__) || defined(TARGET_EMSCRIPTEN)
#define BGD_NO_SDL_DELAY 1
#endif

#if defined(__EMSCRIPTEN__) || defined(TARGET_EMSCRIPTEN) \
 || defined(__ANDROID__) || defined(TARGET_ANDROID) \
 || defined(__SWITCH__) || defined(TARGET_SWITCH) \
 || defined(_arch_dreamcast) || defined(TARGET_DC) \
 || defined(__PSP__) || defined(TARGET_PSP) \
 || defined(__vita__) || defined(TARGET_VITA) \
 || defined(TARGET_TVOS) \
 || defined(__wii__) || defined(TARGET_WII) \
 || defined(TARGET_PANDORA) || defined(TARGET_PS2)
#define BGD_STANDALONE_INTERPRETER 1
#endif

#endif
