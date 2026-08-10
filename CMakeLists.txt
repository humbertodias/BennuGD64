CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

project (BennuGD)
add_definitions (-DVERSION="1.0.0")

# This codebase predates C23; GCC 15+ defaults to C23 where () means (void).
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# GCC 14+ / Clang treat classic C pointer mismatches as errors; keep them as warnings.
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -Wno-error=incompatible-pointer-types
    -Wno-error=int-conversion
  )
endif()

option (STATIC_MODULES "Build modules as static libraries linked into bgdi" ON)
option (INTERPRETER_ONLY "Generate only the bgdi executable" OFF)
option (USE_LIBDES "Use bundled DES instead of OpenSSL" OFF)
option (NO_SOUND "Disable sound support" OFF)
option (BENNUGD_STATIC_DEPS "Prefer statically linked third-party libraries" OFF)

# gcw0 / embedded default remains static modules.
if (STATIC_MODULES)
  add_definitions (-D__STATIC__)
  set (MODULES "STATIC")
else ()
  set (MODULES "SHARED")
endif ()

if (NO_SOUND)
  add_definitions (-DNO_SOUND)
endif ()

if (BENNUGD_STATIC_DEPS)
  if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
    set (CMAKE_FIND_LIBRARY_PREFER_STATIC ON)
  endif ()
  set (OPENSSL_USE_STATIC_LIBS TRUE)
  # Prefer static zlib/libpng when CMake supports the hints (3.24+).
  set (ZLIB_USE_STATIC_LIBS TRUE)
  set (PNG_USE_STATIC_LIBS TRUE)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions (-DTARGET_LINUX)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions (-DTARGET_MAC)
endif()

if (WIN32)
  add_definitions (-DTARGET_WIN32 -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)
endif ()

find_package (SDL3 REQUIRED CONFIG)
if (NOT NO_SOUND)
  find_package (SDL3_mixer REQUIRED CONFIG)
endif ()
find_package (ZLIB REQUIRED)
find_package (PNG REQUIRED)
if (USE_LIBDES)
  add_definitions (-DUSE_LIBDES)
  include_directories (${CMAKE_SOURCE_DIR}/3rdparty/des-4.04b)
else ()
  find_package(OpenSSL REQUIRED)
  add_definitions (-DUSE_OPENSSL)
endif ()

# Temporary: map many SDL2 symbol names while structural API breaks are fixed.
add_definitions (-DSDL_ENABLE_OLD_NAMES)

# Propagate SDL3 headers project-wide (bgdc includes module exports that need SDL.h).
# SDL3::SDL3 may not set INTERFACE_INCLUDE_DIRECTORIES; SDL3::Headers does.
if (TARGET SDL3::Headers)
  get_target_property (_SDL3_INC SDL3::Headers INTERFACE_INCLUDE_DIRECTORIES)
  if (_SDL3_INC)
    include_directories (${_SDL3_INC})
  endif ()
endif ()

# Prefer the static SDL/mixer targets when requested and available.
set (BENNUGD_SDL3_TARGET SDL3::SDL3)
if (BENNUGD_STATIC_DEPS AND TARGET SDL3::SDL3-static)
  set (BENNUGD_SDL3_TARGET SDL3::SDL3-static)
endif ()
set (BENNUGD_SDL3_MIXER_TARGET SDL3_mixer::SDL3_mixer)
if (BENNUGD_STATIC_DEPS AND TARGET SDL3_mixer::SDL3_mixer-static)
  set (BENNUGD_SDL3_MIXER_TARGET SDL3_mixer::SDL3_mixer-static)
endif ()

# Compat helpers (surface format accessors, etc.)
include_directories (
  ${CMAKE_SOURCE_DIR}/modules/libvideo
  ${ZLIB_INCLUDE_DIRS}
)

if (USE_LIBDES)
  add_subdirectory (3rdparty)
endif ()
add_subdirectory (modules)
add_subdirectory (core)
