# Cross-compile Nintendo Switch homebrew with devkitA64 + libnx.
#   cmake --preset switch-aarch64
# Requires DEVKITPRO (typically /opt/devkitpro).

set (CMAKE_SYSTEM_NAME Generic)
set (CMAKE_SYSTEM_VERSION DKA-NX)
set (CMAKE_SYSTEM_PROCESSOR aarch64)

set (SWITCH TRUE)
set (NINTENDO_SWITCH TRUE CACHE BOOL "Build Nintendo Switch homebrew" FORCE)

file (TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)
if (NOT IS_DIRECTORY "${DEVKITPRO}")
  message (FATAL_ERROR "DEVKITPRO is not set or is not a directory (install devkitA64 / libnx)")
endif ()

set (DEVKITA64 "${DEVKITPRO}/devkitA64")
set (LIBNX "${DEVKITPRO}/libnx")
set (PORTLIBS "${DEVKITPRO}/portlibs/switch")

if (NOT EXISTS "${DEVKITA64}/bin/aarch64-none-elf-gcc")
  message (FATAL_ERROR "devkitA64 not found under ${DEVKITA64}")
endif ()
if (NOT EXISTS "${LIBNX}/switch.specs")
  message (FATAL_ERROR "libnx not found under ${LIBNX} (missing switch.specs)")
endif ()

set (CMAKE_C_COMPILER "${DEVKITA64}/bin/aarch64-none-elf-gcc")
set (CMAKE_CXX_COMPILER "${DEVKITA64}/bin/aarch64-none-elf-g++")
set (CMAKE_AR "${DEVKITA64}/bin/aarch64-none-elf-gcc-ar" CACHE FILEPATH "" FORCE)
set (CMAKE_RANLIB "${DEVKITA64}/bin/aarch64-none-elf-gcc-ranlib" CACHE FILEPATH "" FORCE)
set (CMAKE_STRIP "${DEVKITA64}/bin/aarch64-none-elf-strip")
set (CMAKE_NM "${DEVKITA64}/bin/aarch64-none-elf-gcc-nm")

list (APPEND CMAKE_PROGRAM_PATH "${DEVKITPRO}/tools/bin" "${DEVKITA64}/bin")

set (CMAKE_FIND_ROOT_PATH "${DEVKITPRO}" "${DEVKITA64}" "${LIBNX}" "${PORTLIBS}")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set (CMAKE_PREFIX_PATH "${PORTLIBS}" CACHE PATH "Switch portlibs" FORCE)

set_property (GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

set (_bennugd_switch_arch "-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE")
set (CMAKE_C_FLAGS_INIT "-g -Wall -O2 -ffunction-sections -fdata-sections -D_GNU_SOURCE=1 -DNINTENDO_SWITCH -DSWITCH -D__SWITCH__ ${_bennugd_switch_arch}")
set (CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions")
set (CMAKE_EXE_LINKER_FLAGS_INIT "-specs=${LIBNX}/switch.specs -fPIE -L${LIBNX}/lib -L${PORTLIBS}/lib")

set (CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${LIBNX}/include")
set (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "${LIBNX}/include")
set (CMAKE_C_STANDARD_LIBRARIES "-lnx -lm")
set (CMAKE_CXX_STANDARD_LIBRARIES "-lnx -lm -lstdc++")
link_directories ("${LIBNX}/lib" "${PORTLIBS}/lib")

set (ENV{PKG_CONFIG} "${PORTLIBS}/bin/aarch64-none-elf-pkg-config")
set (ENV{PKG_CONFIG_PATH} "${PORTLIBS}/lib/pkgconfig")
set (ENV{PKG_CONFIG_LIBDIR} "${PORTLIBS}/lib/pkgconfig")
set (ENV{PKG_CONFIG_SYSROOT_DIR} "${PORTLIBS}")
if (EXISTS "${PORTLIBS}/bin/aarch64-none-elf-pkg-config")
  set (PKG_CONFIG_EXECUTABLE "${PORTLIBS}/bin/aarch64-none-elf-pkg-config" CACHE FILEPATH "" FORCE)
endif ()
