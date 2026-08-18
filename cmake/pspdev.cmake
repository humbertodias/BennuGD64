# Cross-compile PlayStation Portable homebrew with pspdev.
#   cmake --preset psp-mips
# Requires PSPDEV (typically /usr/local/pspdev) from pspdev/pspdev.

if (DEFINED ENV{PSPDEV} AND IS_DIRECTORY "$ENV{PSPDEV}")
  set (PSPDEV "$ENV{PSPDEV}")
else ()
  set (PSPDEV "/usr/local/pspdev")
  set (ENV{PSPDEV} "${PSPDEV}")
endif ()

if (NOT EXISTS "${PSPDEV}/psp/share/pspdev.cmake")
  message (FATAL_ERROR "pspdev toolchain not found (${PSPDEV}/psp/share/pspdev.cmake)")
endif ()
if (NOT EXISTS "${PSPDEV}/bin/psp-gcc")
  message (FATAL_ERROR "psp-gcc not found (${PSPDEV}/bin/psp-gcc)")
endif ()

include ("${PSPDEV}/psp/share/pspdev.cmake")

# try_compile executables need PSP_MODULE_INFO; test as static archives instead.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (PLATFORM_PSP TRUE CACHE BOOL "Build PlayStation Portable homebrew" FORCE)
set (PSP TRUE CACHE BOOL "Build PlayStation Portable homebrew" FORCE)

# Small GP; newlib has no GNU iconv extras for SDL try_compile.
string (APPEND CMAKE_C_FLAGS_INIT " -G0 -D_GNU_SOURCE=1")
string (APPEND CMAKE_CXX_FLAGS_INIT " -G0 -D_GNU_SOURCE=1 -fno-rtti -fno-exceptions")
string (APPEND CMAKE_C_FLAGS_RELEASE " -O3 -ffast-math")
string (APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -ffast-math")
