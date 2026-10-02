# Cross-compile Zeebo homebrew with the little-endian ARM11 newlib toolchain.
#   cmake --preset zeebo-arm
# Requires ZEEBO_TOOLCHAIN (typically /opt/zeebo) from docker/Dockerfile.zeebo.
# ABI: ARMv6, ARM state, soft-float, little-endian. Shipped BREW modules and
# Zeebx are little-endian; elf2mod writes the MOD header in that byte order.

if (DEFINED ENV{ZEEBO_TOOLCHAIN} AND IS_DIRECTORY "$ENV{ZEEBO_TOOLCHAIN}")
  set (ZEEBO_TOOLCHAIN "$ENV{ZEEBO_TOOLCHAIN}")
else ()
  set (ZEEBO_TOOLCHAIN "/opt/zeebo")
endif ()

set (ZEEBO_HOST "arm-none-eabi")
set (ZEEBO_TRIPLE_PREFIX "${ZEEBO_TOOLCHAIN}/bin/${ZEEBO_HOST}")
set (ZEEBO_SYSROOT "${ZEEBO_TOOLCHAIN}/${ZEEBO_HOST}")

if (NOT EXISTS "${ZEEBO_TRIPLE_PREFIX}-gcc")
  message (FATAL_ERROR "Zeebo gcc not found (${ZEEBO_TRIPLE_PREFIX}-gcc)")
endif ()
if (NOT IS_DIRECTORY "${ZEEBO_SYSROOT}")
  message (FATAL_ERROR "Zeebo sysroot not found (${ZEEBO_SYSROOT})")
endif ()

set (CMAKE_SYSTEM_NAME Generic)
set (CMAKE_SYSTEM_PROCESSOR arm)
set (CMAKE_CROSSCOMPILING TRUE)

set (ZEEBO TRUE CACHE BOOL "Build Zeebo homebrew" FORCE)
set (PLATFORM_ZEEBO TRUE CACHE BOOL "Build Zeebo homebrew" FORCE)

set (CMAKE_C_COMPILER "${ZEEBO_TRIPLE_PREFIX}-gcc")
set (CMAKE_CXX_COMPILER "${ZEEBO_TRIPLE_PREFIX}-g++")
set (CMAKE_AR "${ZEEBO_TRIPLE_PREFIX}-ar" CACHE FILEPATH "" FORCE)
set (CMAKE_RANLIB "${ZEEBO_TRIPLE_PREFIX}-ranlib" CACHE FILEPATH "" FORCE)
set (CMAKE_STRIP "${ZEEBO_TRIPLE_PREFIX}-strip")
set (CMAKE_NM "${ZEEBO_TRIPLE_PREFIX}-nm")

set (CMAKE_SYSROOT "${ZEEBO_SYSROOT}")
set (CMAKE_FIND_ROOT_PATH "${ZEEBO_SYSROOT}" "${ZEEBO_TOOLCHAIN}")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set_property (GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

# ARM1136J-S has no VFP. BREW modules on Zeebo are little-endian.
set (_bennugd_zeebo_arch "-marm -march=armv6 -mtune=arm1136j-s -mfloat-abi=soft")
# arm-none-eabi defaults to AAPCS short enums. SDL3 requires int-sized enums.
set (CMAKE_C_FLAGS_INIT "-O2 -ffunction-sections -fdata-sections -fno-pic -fno-short-enums -D__ZEEBO__=1 -DTARGET_ZEEBO ${_bennugd_zeebo_arch}")
set (CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions")
set (CMAKE_EXE_LINKER_FLAGS_INIT "-specs=nosys.specs -fno-short-enums -Wl,--gc-sections -u _printf_float ${_bennugd_zeebo_arch}")

set (THREADS_PTHREAD_ARG "0" CACHE STRING "" FORCE)
