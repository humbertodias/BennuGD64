CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

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

# param:
# STATIC_MODULES (instead of shared modules)
# INTERPRETER_ONLY (generate only bgdi exec)
# USE_LIBDES (instead OpenSSL, not tested)
# NO_SOUND (not tested)

# gcw0 param to add in builtroot
set (STATIC_MODULES true)
set (INTERPRETER_ONLY false)
set (USE_LIBDES false)
set (NO_SOUND false)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions (-DTARGET_LINUX)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions (-DTARGET_MAC)
endif()

if (STATIC_MODULES)
  add_definitions (-D__STATIC__)
  set (MODULES "STATIC")
else ()
  set (MODULES "SHARED")
endif ()

if (NO_SOUND)
  add_definitions (-DNO_SOUND)
endif ()

find_package (SDL2 REQUIRED CONFIG)
if (NOT NO_SOUND)
  find_package (SDL2_mixer REQUIRED CONFIG)
endif ()
find_package (ZLIB REQUIRED)
find_package (PNG REQUIRED)
if (USE_LIBDES)
  add_definitions (-DUSE_LIBDES)
else ()
  find_package(OpenSSL REQUIRED)
  add_definitions (-DUSE_OPENSSL)
endif ()

# Propagate SDL2 headers the same way the old FindSDL include dir did.
get_target_property (_SDL2_INC SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
if (_SDL2_INC)
  include_directories (${_SDL2_INC})
endif ()
include_directories (
  ${ZLIB_INCLUDE_DIR}
)

if (USE_LIBDES)
  add_subdirectory (3rdparty)
endif ()
add_subdirectory (modules)
add_subdirectory (core)
