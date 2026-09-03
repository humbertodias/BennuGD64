# Cross-compile Sega Dreamcast homebrew with KallistiOS.
#   cmake --preset dreamcast-sh4
# Requires a sourced KOS environment (KOS_BASE, kos-cc). Docker image
# kallistios/dc-kos-toolchain sets this from its entrypoint.

if (NOT DEFINED ENV{KOS_BASE} OR NOT IS_DIRECTORY "$ENV{KOS_BASE}")
  message (FATAL_ERROR "KOS_BASE is not set or is not a directory (source environ.sh)")
endif ()

if (DEFINED ENV{KOS_CMAKE_TOOLCHAIN} AND EXISTS "$ENV{KOS_CMAKE_TOOLCHAIN}")
  include ("$ENV{KOS_CMAKE_TOOLCHAIN}")
elseif (EXISTS "$ENV{KOS_BASE}/utils/cmake/kallistios.toolchain.cmake")
  include ("$ENV{KOS_BASE}/utils/cmake/kallistios.toolchain.cmake")
elseif (EXISTS "$ENV{KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake")
  include ("$ENV{KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake")
else ()
  # nold360/kallistios-sdk (2021) has kos-cc but no CMake toolchain file.
  set (CMAKE_SYSTEM_NAME dreamcast)
  set (CMAKE_SYSTEM_PROCESSOR sh4)
  set (CMAKE_CROSSCOMPILING TRUE)
  set (CMAKE_C_COMPILER "$ENV{KOS_BASE}/utils/gnu_wrappers/kos-cc")
  if (NOT EXISTS "${CMAKE_C_COMPILER}")
    set (CMAKE_C_COMPILER "$ENV{KOS_BASE}/utils/build_wrappers/kos-cc")
  endif ()
  set (CMAKE_CXX_COMPILER "$ENV{KOS_BASE}/utils/gnu_wrappers/kos-c++")
  if (NOT EXISTS "${CMAKE_CXX_COMPILER}")
    set (CMAKE_CXX_COMPILER "$ENV{KOS_BASE}/utils/build_wrappers/kos-c++")
  endif ()
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set_property (GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
endif ()

set (DREAMCAST TRUE CACHE BOOL "Build Sega Dreamcast homebrew" FORCE)
set (PLATFORM_DREAMCAST TRUE CACHE BOOL "Build Sega Dreamcast homebrew" FORCE)

# GPF/SDL does include(kallistios); ship a stub next to this file.
list (INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}")
