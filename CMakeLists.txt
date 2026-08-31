CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

# Must run before project() so the wasi-sdk toolchain can set CMAKE_SYSTEM_NAME.
#   cmake -B build-wasi -DBENNUGD_WASI=ON
#   cmake --build build-wasi --target bgdc
option (BENNUGD_WASI "Cross-compile bgdc to wasm32-wasip1 (downloads wasi-sdk if needed)" OFF)
if (BENNUGD_WASI)
  include (${CMAKE_CURRENT_LIST_DIR}/cmake/FetchWasiSdk.cmake)
endif ()

project (BennuGD)

# This codebase predates C23; GCC 15+ defaults to C23 where () means (void).
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# GCC 14+ / Clang treat classic C pointer mismatches as errors; keep them as warnings.
# Sources are UTF-8 (Spanish/Portuguese comments and copyright).
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -finput-charset=UTF-8
    -Wno-error=incompatible-pointer-types
    -Wno-error=int-conversion
  )
endif ()
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    -Wno-error=pointer-to-int-cast
    -Wno-error=int-to-pointer-cast
  )
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -Wno-error=void-pointer-to-int-cast
    -Wno-error=int-to-void-pointer-cast
    -Wno-error=pointer-to-int-cast
  )
elseif (MSVC)
  add_compile_options(/utf-8)
endif()

# osxcross: SDL @available needs compiler-rt. Do not link_libraries() globally —
# SDL3's export(SDL3-static) would then require this target in an export set.
if (APPLE AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND NOT BENNUGD_OSXCROSS_COMPILER_RT)
  add_library (bennugd_osxcross_rt STATIC cmake/osxcross_isplatformversion.c)
  set_target_properties (bennugd_osxcross_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()

option (STATIC_MODULES "Build modules as static libraries linked into bgdi" ON)
option (INTERPRETER_ONLY "Generate only the bgdi executable" OFF)
option (COMPILER_ONLY "Generate only the bgdc executable" OFF)
option (USE_LIBDES "Use bundled DES instead of OpenSSL" OFF)
option (NO_SOUND "Disable sound support" OFF)

if (EMSCRIPTEN)
  include (${CMAKE_SOURCE_DIR}/cmake/Emscripten.cmake)
endif ()
if (ANDROID)
  include (${CMAKE_SOURCE_DIR}/cmake/Android.cmake)
endif ()
if (NINTENDO_SWITCH)
  include (${CMAKE_SOURCE_DIR}/cmake/NintendoSwitch.cmake)
endif ()
if (PLATFORM_DREAMCAST OR DREAMCAST)
  include (${CMAKE_SOURCE_DIR}/cmake/Dreamcast.cmake)
endif ()
if (PLATFORM_PSP OR PSP)
  include (${CMAKE_SOURCE_DIR}/cmake/PSP.cmake)
endif ()
if (PLATFORM_VITA OR VITA)
  include (${CMAKE_SOURCE_DIR}/cmake/PlayStationVita.cmake)
endif ()
if (PLATFORM_PS2 OR PS2)
  include (${CMAKE_SOURCE_DIR}/cmake/PlayStation2.cmake)
endif ()
if (PLATFORM_PS3 OR PS3)
  include (${CMAKE_SOURCE_DIR}/cmake/PlayStation3.cmake)
endif ()
if (PLATFORM_PANDORA OR OPENPANDORA)
  include (${CMAKE_SOURCE_DIR}/cmake/Pandora.cmake)
endif ()
if (NINTENDO_WII OR PLATFORM_WII)
  include (${CMAKE_SOURCE_DIR}/cmake/NintendoWii.cmake)
endif ()
if (CMAKE_SYSTEM_NAME MATCHES "WASI" OR CMAKE_C_COMPILER_TARGET MATCHES "wasm32-wasi")
  include (${CMAKE_SOURCE_DIR}/cmake/WASI.cmake)
endif ()
option (BENNUGD_BUNDLE_DEPS "Download and build zlib, libpng, SDL3 and SDL3_mixer via CMake FetchContent" ON)
option (BENNUGD_STATIC_DEPS "Prefer statically linked third-party libraries" OFF)
set (BENNUGD_VERSION "" CACHE STRING "Release version (empty = git tag such as 1.2.3, else git describe)")

# gcw0 / embedded default remains static modules.
if (STATIC_MODULES)
  add_definitions (-D__STATIC__)
  set (MODULES "STATIC")
else ()
  set (MODULES "SHARED")
  # Static in-tree libs (e.g. des) and objects must be PIC to link into .so/.dylib.
  set (CMAKE_POSITION_INDEPENDENT_CODE ON)
  # Shared modules resolve next to themselves; libbgdrtm resolves next to bgdi
  # and one directory up from modules/.
  if (NOT WIN32)
    if (APPLE)
      set (_bennugd_rpath "@loader_path;@loader_path/modules;@loader_path/..")
    else ()
      set (_bennugd_rpath "\$ORIGIN;\$ORIGIN/modules;\$ORIGIN/..")
    endif ()
    set (CMAKE_BUILD_RPATH "${_bennugd_rpath}")
    set (CMAKE_INSTALL_RPATH "${_bennugd_rpath}")
    set (CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
  endif ()
endif ()

if (NO_SOUND)
  add_definitions (-DNO_SOUND)
endif ()

if (BENNUGD_BUNDLE_DEPS)
  set (BENNUGD_STATIC_DEPS ON)
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

if (EMSCRIPTEN)
  add_definitions (-DTARGET_EMSCRIPTEN)
elseif (ANDROID)
  add_definitions (-DTARGET_ANDROID)
elseif (NINTENDO_SWITCH)
  add_definitions (-DTARGET_SWITCH)
elseif (PLATFORM_DREAMCAST OR DREAMCAST)
  add_definitions (-DTARGET_DC)
elseif (PLATFORM_PSP OR PSP)
  add_definitions (-DTARGET_PSP)
elseif (PLATFORM_VITA OR VITA)
  add_definitions (-DTARGET_VITA)
elseif (PLATFORM_PS2 OR PS2)
  add_definitions (-DTARGET_PS2)
elseif (PLATFORM_PS3 OR PS3)
  add_definitions (-DTARGET_PS3)
elseif (PLATFORM_PANDORA OR OPENPANDORA)
  add_definitions (-DTARGET_PANDORA)
elseif (NINTENDO_WII OR PLATFORM_WII)
  add_definitions (-DTARGET_WII)
elseif (CMAKE_SYSTEM_NAME MATCHES "WASI")
  add_definitions (-DTARGET_WASI)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions (-DTARGET_LINUX)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions (-DTARGET_MAC)
endif()

if (WIN32)
  add_definitions (-DTARGET_WIN32 -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)
endif ()

if (MINGW)
  add_link_options (-static-libgcc)
endif ()

if (MSVC)
  set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

if (BENNUGD_BUNDLE_DEPS)
  include (${CMAKE_SOURCE_DIR}/cmake/FetchDeps.cmake)
else ()
  if (NOT COMPILER_ONLY)
    find_package (SDL3 REQUIRED CONFIG)
    if (NOT NO_SOUND)
      find_package (SDL3_mixer REQUIRED CONFIG)
    endif ()
    find_package (PNG REQUIRED)
  endif ()
  find_package (ZLIB REQUIRED)
endif ()

if (USE_LIBDES)
  add_definitions (-DUSE_LIBDES)
  include_directories (${CMAKE_SOURCE_DIR}/3rdparty/des-4.04b)
else ()
  find_package(OpenSSL REQUIRED)
  add_definitions (-DUSE_OPENSSL)
endif ()

# Temporary: map many SDL2 symbol names while structural API breaks are fixed.
if (NOT COMPILER_ONLY)
  add_definitions (-DSDL_ENABLE_OLD_NAMES)
endif ()

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
if (NOT ANDROID AND BENNUGD_STATIC_DEPS AND TARGET SDL3::SDL3-static)
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

set (BENNUGD_GIT_HEADER "${CMAKE_BINARY_DIR}/generated/bennugd_git.h")
execute_process (
  COMMAND "${CMAKE_COMMAND}"
    -DGIT_DIR=${CMAKE_SOURCE_DIR}
    -DOUT=${BENNUGD_GIT_HEADER}
    -DBENNUGD_VERSION=${BENNUGD_VERSION}
    -P ${CMAKE_SOURCE_DIR}/cmake/git_id.cmake
)
add_custom_target (
  bennugd_git_header
  COMMAND "${CMAKE_COMMAND}"
    -DGIT_DIR=${CMAKE_SOURCE_DIR}
    -DOUT=${BENNUGD_GIT_HEADER}
    -DBENNUGD_VERSION=${BENNUGD_VERSION}
    -P ${CMAKE_SOURCE_DIR}/cmake/git_id.cmake
  BYPRODUCTS ${BENNUGD_GIT_HEADER}
  COMMENT "Updating version for banners and window title"
)
include_directories (${CMAKE_BINARY_DIR}/generated)
include (${CMAKE_BINARY_DIR}/generated/bennugd_version.cmake)
set (BENNUGD_PACKAGE_VERSION "${BENNUGD_RESOLVED_VERSION}")

if (USE_LIBDES)
  add_subdirectory (3rdparty)
endif ()
if (NOT COMPILER_ONLY)
  add_subdirectory (modules)
endif ()
add_subdirectory (core)
include (${CMAKE_SOURCE_DIR}/cmake/InstallLayout.cmake)
include (${CMAKE_SOURCE_DIR}/cmake/SmokeTests.cmake)

# Shared .dylibs that pull in libSDL3.a need the same @available stub as bgdi.
if (TARGET bennugd_osxcross_rt)
  foreach (_tgt
      bgdi bgdc bgdrtm
      sdlhandler video render key joy mouse wm scroll bgload
      mod_sound mod_map mod_wpad mod_wm mod_video mod_timers mod_time
      mod_flic mod_debug)
    if (TARGET ${_tgt})
      target_link_libraries (${_tgt} bennugd_osxcross_rt)
    endif ()
  endforeach ()
endif ()
