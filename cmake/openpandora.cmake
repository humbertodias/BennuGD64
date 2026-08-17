# Cross-compile OpenPandora homebrew with the Ångström ARM toolchain.
#   cmake --preset pandora-arm
# Requires TOOLCHAIN (typically /opt/openpandora) from
# scummvm/dockerized-toolchains:openpandora.

if (DEFINED ENV{TOOLCHAIN} AND IS_DIRECTORY "$ENV{TOOLCHAIN}")
  set (PANDORA_TOOLCHAIN "$ENV{TOOLCHAIN}")
else ()
  set (PANDORA_TOOLCHAIN "/opt/openpandora")
endif ()

set (PANDORA_HOST "arm-angstrom-linux-gnueabi")
set (PANDORA_TRIPLE_PREFIX "${PANDORA_TOOLCHAIN}/bin/${PANDORA_HOST}")
set (PANDORA_SYSROOT "${PANDORA_TOOLCHAIN}/${PANDORA_HOST}/sysroot")

if (NOT EXISTS "${PANDORA_TRIPLE_PREFIX}-gcc")
  message (FATAL_ERROR "OpenPandora gcc not found (${PANDORA_TRIPLE_PREFIX}-gcc)")
endif ()
if (NOT IS_DIRECTORY "${PANDORA_SYSROOT}")
  message (FATAL_ERROR "OpenPandora sysroot not found (${PANDORA_SYSROOT})")
endif ()

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR arm)

set (PANDORA TRUE CACHE BOOL "Build OpenPandora homebrew" FORCE)
set (PLATFORM_PANDORA TRUE CACHE BOOL "Build OpenPandora homebrew" FORCE)
set (OPENPANDORA TRUE CACHE BOOL "Build OpenPandora homebrew" FORCE)

set (CMAKE_SYSROOT "${PANDORA_SYSROOT}")
set (CMAKE_C_COMPILER "${PANDORA_TRIPLE_PREFIX}-gcc")
set (CMAKE_CXX_COMPILER "${PANDORA_TRIPLE_PREFIX}-g++")
set (CMAKE_AR "${PANDORA_TRIPLE_PREFIX}-ar" CACHE FILEPATH "" FORCE)
set (CMAKE_RANLIB "${PANDORA_TRIPLE_PREFIX}-ranlib" CACHE FILEPATH "" FORCE)
set (CMAKE_STRIP "${PANDORA_TRIPLE_PREFIX}-strip")
set (CMAKE_NM "${PANDORA_TRIPLE_PREFIX}-nm")

list (APPEND CMAKE_PROGRAM_PATH "${PANDORA_TOOLCHAIN}/bin")

set (CMAKE_FIND_ROOT_PATH "${PANDORA_SYSROOT}" "${PANDORA_TOOLCHAIN}")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Cortex-A8 / OMAP3530. Sysroot is soft-float AAPCS with VFPv3 (softfp).
set (_bennugd_pandora_arch "-march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp")
set (CMAKE_C_FLAGS_INIT "-O2 -ffunction-sections -fdata-sections -D_GNU_SOURCE=1 -DTARGET_PANDORA -DARM ${_bennugd_pandora_arch}")
set (CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions")
set (CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--as-needed -Wl,--gc-sections")

set (ENV{PKG_CONFIG_SYSROOT_DIR} "${PANDORA_SYSROOT}")
set (ENV{PKG_CONFIG_LIBDIR} "${PANDORA_SYSROOT}/usr/lib/pkgconfig:${PANDORA_SYSROOT}/usr/share/pkgconfig")
set (ENV{PKG_CONFIG_PATH} "")

set (THREADS_PTHREAD_ARG "0" CACHE STRING "" FORCE)
set (CMAKE_THREAD_LIBS_INIT "-lpthread")
set (CMAKE_HAVE_THREADS_LIBRARY 1)
set (CMAKE_USE_PTHREADS_INIT 1)
