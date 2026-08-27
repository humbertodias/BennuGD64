# Cross-compile PlayStation 2 homebrew with ps2dev.
#   cmake --preset ps2-mips
# Requires PS2DEV (typically /usr/local/ps2dev) from ps2dev/ps2dev.

if (DEFINED ENV{PS2DEV} AND IS_DIRECTORY "$ENV{PS2DEV}")
  set (PS2DEV "$ENV{PS2DEV}")
else ()
  set (PS2DEV "/usr/local/ps2dev")
  set (ENV{PS2DEV} "${PS2DEV}")
endif ()

if (IS_DIRECTORY "${PS2DEV}/ps2sdk")
  set (ENV{PS2SDK} "${PS2DEV}/ps2sdk")
endif ()
if (IS_DIRECTORY "${PS2DEV}/gsKit")
  set (ENV{GSKIT} "${PS2DEV}/gsKit")
endif ()

set (_bennugd_ps2_toolchain "")
foreach ( _cand
    "${PS2DEV}/share/ps2dev.cmake"
    "${PS2DEV}/ps2sdk/ps2dev.cmake"
    "$ENV{PS2SDK}/ps2dev.cmake"
)
  if (EXISTS "${_cand}")
    set (_bennugd_ps2_toolchain "${_cand}")
    break ()
  endif ()
endforeach ()

if (_bennugd_ps2_toolchain STREQUAL "")
  message (FATAL_ERROR "ps2dev toolchain not found (${PS2DEV}/share/ps2dev.cmake)")
endif ()
if (NOT EXISTS "${PS2DEV}/ee/bin/mips64r5900el-ps2-elf-gcc"
    AND NOT EXISTS "${PS2DEV}/ee/bin/ee-gcc")
  message (FATAL_ERROR "ps2dev EE gcc not found under ${PS2DEV}/ee/bin")
endif ()

include ("${_bennugd_ps2_toolchain}")

# try_compile executables need PS2 CRT/linkfile; test as static archives instead.
# SDL libc/atomic checks then false-positive; FetchDeps forces those OFF for PS2.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (PLATFORM_PS2 TRUE CACHE BOOL "Build PlayStation 2 homebrew" FORCE)
set (PS2 TRUE CACHE BOOL "Build PlayStation 2 homebrew" FORCE)

string (APPEND CMAKE_C_FLAGS_INIT " -D_GNU_SOURCE=1")
string (APPEND CMAKE_CXX_FLAGS_INIT " -D_GNU_SOURCE=1 -fno-rtti -fno-exceptions")
string (APPEND CMAKE_C_FLAGS_RELEASE " -O3 -ffast-math")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -ffast-math")
