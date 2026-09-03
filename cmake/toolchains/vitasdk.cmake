# Cross-compile PlayStation Vita homebrew with vitasdk.
#   cmake --preset vita-arm
# Requires VITASDK (typically /usr/local/vitasdk) from vitasdk/vitasdk.

if (DEFINED ENV{VITASDK} AND IS_DIRECTORY "$ENV{VITASDK}")
  set (VITASDK "$ENV{VITASDK}")
else ()
  set (VITASDK "/usr/local/vitasdk")
  set (ENV{VITASDK} "${VITASDK}")
endif ()

if (NOT EXISTS "${VITASDK}/share/vita.toolchain.cmake")
  message (FATAL_ERROR "vitasdk toolchain not found (${VITASDK}/share/vita.toolchain.cmake)")
endif ()
if (NOT EXISTS "${VITASDK}/bin/arm-vita-eabi-gcc")
  message (FATAL_ERROR "arm-vita-eabi-gcc not found (${VITASDK}/bin/arm-vita-eabi-gcc)")
endif ()

include ("${VITASDK}/share/vita.toolchain.cmake")

# try_compile executables cannot run on the host.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (PLATFORM_VITA TRUE CACHE BOOL "Build PlayStation Vita homebrew" FORCE)
set (VITA TRUE CACHE BOOL "Build PlayStation Vita homebrew" FORCE)

string (APPEND CMAKE_C_FLAGS_INIT " -D_GNU_SOURCE=1")
string (APPEND CMAKE_CXX_FLAGS_INIT " -D_GNU_SOURCE=1 -fno-rtti -fno-exceptions")
string (APPEND CMAKE_C_FLAGS_RELEASE " -O3 -ffast-math -ftree-vectorize -mfpu=neon")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -ffast-math -ftree-vectorize -mfpu=neon")
