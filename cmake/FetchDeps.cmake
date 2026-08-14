# Download and build zlib, libpng, SDL3 and SDL3_mixer as static PIC libraries.
# Used when BENNUGD_BUNDLE_DEPS=ON so a single CMake configure/build is enough.

include (FetchContent)

if (POLICY CMP0135)
  cmake_policy (SET CMP0135 NEW)
endif ()

set (BENNUGD_ZLIB_VERSION "1.3.1" CACHE STRING "zlib version to fetch")
set (BENNUGD_LIBPNG_VERSION "1.6.47" CACHE STRING "libpng version to fetch")
set (BENNUGD_SDL3_REF "release-3.4.14" CACHE STRING "SDL3 git tag/branch to fetch")
set (BENNUGD_SDL3_MIXER_REF "release-3.2.4" CACHE STRING "SDL3_mixer git tag/branch to fetch")

# Static archives must be PIC so they can later link into .so/.dylib modules.
if (NOT EMSCRIPTEN)
  set (CMAKE_POSITION_INDEPENDENT_CODE ON)
  if (NOT MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif ()
endif ()

set (_bennugd_saved_shared "${BUILD_SHARED_LIBS}")
set (BUILD_SHARED_LIBS OFF)

# Fetched deps are linked in-tree; do not generate their install/export rules.
# libpng's install(EXPORT) otherwise requires zlibstatic in an export set.
set (_bennugd_saved_skip_install "${CMAKE_SKIP_INSTALL_RULES}")
set (CMAKE_SKIP_INSTALL_RULES ON)
set (SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_EXPORT ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_LIBRARIES ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_HEADERS ON CACHE BOOL "" FORCE)
set (SKIP_INSTALL_FILES ON CACHE BOOL "" FORCE)

# --- zlib ---
set (ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare (
  zlib
  URL "https://github.com/madler/zlib/releases/download/v${BENNUGD_ZLIB_VERSION}/zlib-${BENNUGD_ZLIB_VERSION}.tar.gz"
)
FetchContent_GetProperties (zlib)
if (NOT zlib_POPULATED)
  FetchContent_Populate (zlib)
  add_subdirectory (${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (NOT TARGET ZLIB::ZLIB)
  if (TARGET zlibstatic)
    add_library (ZLIB::ZLIB ALIAS zlibstatic)
  elseif (TARGET zlib)
    add_library (ZLIB::ZLIB ALIAS zlib)
  else ()
    message (FATAL_ERROR "Bundled zlib did not create zlibstatic or zlib")
  endif ()
endif ()
set (ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}")
set (ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
set (ZLIB_FOUND TRUE)

# libpng calls find_package(ZLIB); prefer our in-tree target over the module.
file (WRITE "${CMAKE_BINARY_DIR}/cmake-overrides/FindZLIB.cmake"
"if (TARGET ZLIB::ZLIB)
  set (ZLIB_FOUND TRUE)
  set (ZLIB_INCLUDE_DIR \"${zlib_SOURCE_DIR}\" \"${zlib_BINARY_DIR}\")
  set (ZLIB_INCLUDE_DIRS \"${zlib_SOURCE_DIR}\" \"${zlib_BINARY_DIR}\")
  set (ZLIB_LIBRARIES ZLIB::ZLIB)
  set (ZLIB_LIBRARY ZLIB::ZLIB)
  return ()
endif ()
")
list (PREPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/cmake-overrides")

# --- libpng ---
set (PNG_SHARED OFF CACHE BOOL "" FORCE)
set (PNG_STATIC ON CACHE BOOL "" FORCE)
set (PNG_FRAMEWORK OFF CACHE BOOL "" FORCE)
set (PNG_TESTS OFF CACHE BOOL "" FORCE)
set (PNG_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_Declare (
  libpng
  URL "https://github.com/pnggroup/libpng/archive/refs/tags/v${BENNUGD_LIBPNG_VERSION}.tar.gz"
)
FetchContent_GetProperties (libpng)
if (NOT libpng_POPULATED)
  FetchContent_Populate (libpng)
  add_subdirectory (${libpng_SOURCE_DIR} ${libpng_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (NOT TARGET PNG::PNG)
  if (TARGET png_static)
    add_library (PNG::PNG ALIAS png_static)
  elseif (TARGET png)
    add_library (PNG::PNG ALIAS png)
  else ()
    message (FATAL_ERROR "Bundled libpng did not create png_static or png")
  endif ()
endif ()

# --- SDL3 ---
set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)
set (SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set (SDL_TESTS OFF CACHE BOOL "" FORCE)
set (SDL_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set (SDL_LIBUSB OFF CACHE BOOL "" FORCE)
set (SDL_HIDAPI_LIBUSB OFF CACHE BOOL "" FORCE)
FetchContent_Declare (
  sdl3
  GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
  GIT_TAG "${BENNUGD_SDL3_REF}"
  GIT_SHALLOW TRUE
)
FetchContent_GetProperties (sdl3)
if (NOT sdl3_POPULATED)
  FetchContent_Populate (sdl3)
  add_subdirectory (${sdl3_SOURCE_DIR} ${sdl3_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (EXISTS "${sdl3_BINARY_DIR}/SDL3Config.cmake")
  set (SDL3_DIR "${sdl3_BINARY_DIR}" CACHE PATH "" FORCE)
elseif (EXISTS "${sdl3_BINARY_DIR}/cmake/SDL3Config.cmake")
  set (SDL3_DIR "${sdl3_BINARY_DIR}/cmake" CACHE PATH "" FORCE)
endif ()
list (PREPEND CMAKE_PREFIX_PATH "${sdl3_BINARY_DIR}")

# --- SDL3_mixer ---
if (NOT NO_SOUND)
  set (SDLMIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_EXAMPLES OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_TESTS OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_STRICT OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_GME OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_MOD OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_MIDI OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_FLAC OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_OPUS OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_MP3 ON CACHE BOOL "" FORCE)
  set (SDLMIXER_MP3_DRMP3 ON CACHE BOOL "" FORCE)
  set (SDLMIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
  set (SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
  set (SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "" FORCE)
  FetchContent_Declare (
    sdl3_mixer
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_mixer.git"
    GIT_TAG "${BENNUGD_SDL3_MIXER_REF}"
    GIT_SHALLOW TRUE
  )
  FetchContent_GetProperties (sdl3_mixer)
  if (NOT sdl3_mixer_POPULATED)
    FetchContent_Populate (sdl3_mixer)
    if (MSVC)
      set (_stb "${sdl3_mixer_SOURCE_DIR}/src/decoder_stb_vorbis.c")
      if (EXISTS "${_stb}")
        file (READ "${_stb}" _stb_txt)
        string (REPLACE
          "#define STB_FORCEINLINE SDL_FORCE_INLINE"
          "#define STB_FORCEINLINE static SDL_FORCE_INLINE"
          _stb_txt "${_stb_txt}")
        file (WRITE "${_stb}" "${_stb_txt}")
      endif ()
    endif ()
    add_subdirectory (${sdl3_mixer_SOURCE_DIR} ${sdl3_mixer_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif ()
endif ()

set (BUILD_SHARED_LIBS "${_bennugd_saved_shared}")
set (CMAKE_SKIP_INSTALL_RULES "${_bennugd_saved_skip_install}")
