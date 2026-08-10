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
set (INTERPRETER_ONLY true)
set (USE_LIBDES false)
set (NO_SOUND false)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions (-DTARGET_LINUX)
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

find_package (SDL REQUIRED)
find_package (ZLIB REQUIRED)
if (USE_LIBDES)
  add_definitions (-DUSE_LIBDES)
else ()
  find_package(OpenSSL REQUIRED)
  add_definitions (-DUSE_OPENSSL)
endif ()

include_directories (
  ${SDL_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
)

if (USE_LIBDES)
  add_subdirectory (3rdparty)
endif ()
add_subdirectory (modules)
add_subdirectory (core)
