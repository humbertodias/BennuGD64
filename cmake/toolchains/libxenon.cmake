# Cross-compile Xbox 360 homebrew with the open Free60/libXenon toolchain.
#   cmake --preset xbox360-powerpc
# Requires DEVKITXENON (normally /usr/local/xenon).

if (DEFINED ENV{DEVKITXENON} AND IS_DIRECTORY "$ENV{DEVKITXENON}")
  set (DEVKITXENON "$ENV{DEVKITXENON}")
else ()
  set (DEVKITXENON "/usr/local/xenon")
  set (ENV{DEVKITXENON} "${DEVKITXENON}")
endif ()

set (CMAKE_SYSTEM_NAME Generic)
set (CMAKE_SYSTEM_VERSION 1)
set (CMAKE_SYSTEM_PROCESSOR powerpc64)
set (CMAKE_CROSSCOMPILING TRUE)

if (NOT EXISTS "${DEVKITXENON}/bin/xenon-gcc")
  message (FATAL_ERROR "libXenon compiler not found at ${DEVKITXENON}/bin/xenon-gcc")
endif ()

set (CMAKE_C_COMPILER "${DEVKITXENON}/bin/xenon-gcc")
set (CMAKE_CXX_COMPILER "${DEVKITXENON}/bin/xenon-g++")
set (CMAKE_AR "${DEVKITXENON}/bin/xenon-ar" CACHE FILEPATH "" FORCE)
set (CMAKE_RANLIB "${DEVKITXENON}/bin/xenon-ranlib" CACHE FILEPATH "" FORCE)
set (CMAKE_STRIP "${DEVKITXENON}/bin/xenon-strip")
set (CMAKE_OBJCOPY "${DEVKITXENON}/bin/xenon-objcopy")

set (CMAKE_FIND_ROOT_PATH "${DEVKITXENON}" "${DEVKITXENON}/usr")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set_property (GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

set (_xenon_arch "-m32 -maltivec -fno-pic -mpowerpc64 -mhard-float")
set (_xenon_platform "-DXENON=1 -D__XBOX360__=1 -Ulinux -U__linux -U__linux__ -Uunix -U__unix -U__unix__")
string (APPEND CMAKE_C_FLAGS_INIT
  " ${_xenon_platform} -D_GNU_SOURCE=1 ${_xenon_arch} -ffunction-sections -fdata-sections")
string (APPEND CMAKE_CXX_FLAGS_INIT
  " ${_xenon_platform} -D_GNU_SOURCE=1 ${_xenon_arch} -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions")
string (APPEND CMAKE_EXE_LINKER_FLAGS_INIT
  " ${_xenon_arch} -Wl,--gc-sections -Wl,-n -Wl,-T,${DEVKITXENON}/app.lds -L${DEVKITXENON}/xenon/lib/32 -L${DEVKITXENON}/usr/lib")

set (CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${DEVKITXENON}/usr/include")
set (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "${DEVKITXENON}/usr/include")

set (PLATFORM_XBOX360 TRUE CACHE BOOL "Build Xbox 360 libXenon homebrew" FORCE)
set (XBOX360 TRUE CACHE BOOL "Build Xbox 360 libXenon homebrew" FORCE)
