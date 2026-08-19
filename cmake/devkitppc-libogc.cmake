# Cross-compile Nintendo Wii homebrew with devkitPPC + libogc.
#   cmake --preset wii-powerpc
# Requires DEVKITPRO (typically /opt/devkitpro).
# Prefers the official /opt/devkitpro/cmake/Wii.cmake when present.

file (TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)
if (NOT IS_DIRECTORY "${DEVKITPRO}")
  message (FATAL_ERROR "DEVKITPRO is not set or is not a directory (install devkitPPC / libogc)")
endif ()

if (EXISTS "${DEVKITPRO}/cmake/Wii.cmake")
  include ("${DEVKITPRO}/cmake/Wii.cmake")
else ()
  set (CMAKE_SYSTEM_NAME NintendoWii)
  set (CMAKE_SYSTEM_VERSION DKP-WII)
  set (CMAKE_SYSTEM_PROCESSOR powerpc)

  set (DEVKITPPC "${DEVKITPRO}/devkitPPC")
  set (LIBOGC "${DEVKITPRO}/libogc")
  set (PORTLIBS "${DEVKITPRO}/portlibs/wii")

  if (NOT EXISTS "${DEVKITPPC}/bin/powerpc-eabi-gcc")
    message (FATAL_ERROR "devkitPPC not found under ${DEVKITPPC}")
  endif ()
  if (NOT IS_DIRECTORY "${LIBOGC}/include")
    message (FATAL_ERROR "libogc not found under ${LIBOGC}")
  endif ()

  set (CMAKE_C_COMPILER "${DEVKITPPC}/bin/powerpc-eabi-gcc")
  set (CMAKE_CXX_COMPILER "${DEVKITPPC}/bin/powerpc-eabi-g++")
  set (CMAKE_AR "${DEVKITPPC}/bin/powerpc-eabi-gcc-ar" CACHE FILEPATH "" FORCE)
  set (CMAKE_RANLIB "${DEVKITPPC}/bin/powerpc-eabi-gcc-ranlib" CACHE FILEPATH "" FORCE)
  set (CMAKE_STRIP "${DEVKITPPC}/bin/powerpc-eabi-strip")
  set (CMAKE_NM "${DEVKITPPC}/bin/powerpc-eabi-gcc-nm")

  list (APPEND CMAKE_PROGRAM_PATH "${DEVKITPRO}/tools/bin" "${DEVKITPPC}/bin")

  set (CMAKE_FIND_ROOT_PATH "${DEVKITPRO}" "${DEVKITPPC}" "${LIBOGC}" "${PORTLIBS}")
  set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

  set (CMAKE_PREFIX_PATH "${PORTLIBS}" CACHE PATH "Wii portlibs" FORCE)

  set (OGC TRUE)
  set (OGC_ARCH_SETTINGS "-mrvl -mcpu=750 -meabi -mhard-float")
  set (OGC_LINKER_FLAGS "-L${LIBOGC}/lib/wii")

  set_property (GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

  set (CMAKE_C_FLAGS_INIT "-g -Wall -O2 -ffunction-sections -fdata-sections -DGEKKO -DHW_RVL -D__wii__ -D_GNU_SOURCE=1 ${OGC_ARCH_SETTINGS}")
  set (CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-rtti -fno-exceptions")
  set (CMAKE_EXE_LINKER_FLAGS_INIT "${OGC_ARCH_SETTINGS} ${OGC_LINKER_FLAGS}")

  set (CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${LIBOGC}/include")
  set (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "${LIBOGC}/include")
  set (CMAKE_C_STANDARD_LIBRARIES "-lwiiuse -lbte -lwiikeyboard -laesnd -lfat -logc -lm")
  set (CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} -lstdc++")
  link_directories ("${LIBOGC}/lib/wii" "${PORTLIBS}/lib")

  if (EXISTS "${PORTLIBS}/bin/powerpc-eabi-pkg-config")
    set (PKG_CONFIG_EXECUTABLE "${PORTLIBS}/bin/powerpc-eabi-pkg-config" CACHE FILEPATH "" FORCE)
  endif ()
  set (ENV{PKG_CONFIG_PATH} "${PORTLIBS}/lib/pkgconfig")
  set (ENV{PKG_CONFIG_LIBDIR} "${PORTLIBS}/lib/pkgconfig")
endif ()

# try_compile executables need Wii CRT/specs; test as static archives instead.
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set (NINTENDO_WII TRUE CACHE BOOL "Build Nintendo Wii homebrew" FORCE)
set (PLATFORM_WII TRUE CACHE BOOL "Build Nintendo Wii homebrew" FORCE)
set (OGC TRUE CACHE BOOL "libogc / GameCube-Wii platform" FORCE)
