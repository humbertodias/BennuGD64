# PlayStation 3 homebrew: static modules, interpreter only (bgdi.elf → EBOOT.BIN / PKG).
# Compile .prg on a host with the ps3-host preset.

set (STATIC_MODULES ON CACHE BOOL "PS3 homebrew is a single PKG" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "PKG ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)
set (SDL_TEST OFF CACHE BOOL "" FORCE)
set (SDL_TESTS OFF CACHE BOOL "" FORCE)
set (SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set (SDL_GPU OFF CACHE BOOL "" FORCE)
set (SDL_CAMERA OFF CACHE BOOL "" FORCE)
set (SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set (SDL_HIDAPI OFF CACHE BOOL "" FORCE)
set (SDL_SENSOR OFF CACHE BOOL "" FORCE)
set (SDL_DIALOG OFF CACHE BOOL "" FORCE)
set (SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
set (SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "PS3 builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for ps3dev)")
endif ()

if (DEFINED ENV{PS3DEV} AND IS_DIRECTORY "$ENV{PS3DEV}")
  link_directories (
    "$ENV{PS3DEV}/ppu/lib"
    "$ENV{PS3DEV}/ppu/ppu/lib"
    "$ENV{PS3DEV}/portlibs/ppu/lib"
  )
  # Drop portlibs -I from a cached first configure (ps3dev.cmake used to
  # put it in CMAKE_C_FLAGS_INIT and poison libpng's zlib.h).
  foreach (_ps3_flags CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS_INIT CMAKE_CXX_FLAGS_INIT)
    if (DEFINED ${_ps3_flags})
      string (REGEX REPLACE "[ \t]*-I[^ \t]*portlibs/ppu/include" "" ${_ps3_flags} "${${_ps3_flags}}")
    endif ()
  endforeach ()
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
endif ()

# Cached CMAKE_C_FLAGS from a prior configure will not pick up toolchain
# FLAGS_INIT. Keep the CELL-style PSL1GHT aliases on the PPU include path.
set (_ps3_compat "${CMAKE_SOURCE_DIR}/cmake/ps3-compat")
if (IS_DIRECTORY "${_ps3_compat}" AND NOT CMAKE_C_FLAGS MATCHES "ps3-compat")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${_ps3_compat}" CACHE STRING "" FORCE)
endif ()
if (IS_DIRECTORY "${_ps3_compat}" AND NOT CMAKE_CXX_FLAGS MATCHES "ps3-compat")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${_ps3_compat}" CACHE STRING "" FORCE)
endif ()

# Host zlib's CMake renames zconf.h in the source tree. A previous PPU
# configure with FETCHCONTENT_SOURCE_DIR_ZLIB still has zlib_SOURCE_DIR
# pointing at build-ps3-host/_deps/zlib-src (no zconf.h).
unset (FETCHCONTENT_SOURCE_DIR_ZLIB CACHE)
unset (FETCHCONTENT_SOURCE_DIR_ZLIB)
if (DEFINED zlib_SOURCE_DIR)
  file (TO_CMAKE_PATH "${zlib_SOURCE_DIR}" _bennugd_zlib_src)
  file (TO_CMAKE_PATH "${CMAKE_BINARY_DIR}/" _bennugd_ppu_root)
  string (FIND "${_bennugd_zlib_src}" "${_bennugd_ppu_root}" _bennugd_zlib_off)
  if (NOT _bennugd_zlib_off EQUAL 0)
    unset (zlib_SOURCE_DIR CACHE)
    unset (zlib_BINARY_DIR CACHE)
    unset (zlib_POPULATED CACHE)
    unset (zlib_SOURCE_DIR)
    unset (zlib_BINARY_DIR)
    unset (zlib_POPULATED)
  endif ()
endif ()

# Call after include(FetchDeps). png_static already exists so these -I flags
# do not go on libpng. Engine targets created afterwards need zlib.h.
macro (bennugd_ps3_engine_zlib_includes)
  set (_ps3_zincs "")
  if (_bennugd_skip_fetch_zlib_png AND DEFINED ENV{PS3DEV})
    set (_ps3_zincs "$ENV{PS3DEV}/portlibs/ppu/include")
  elseif (DEFINED zlib_SOURCE_DIR AND NOT zlib_SOURCE_DIR STREQUAL "")
    set (_ps3_zincs "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
  else ()
    set (_ps3_zincs
      "${CMAKE_BINARY_DIR}/_deps/zlib-src"
      "${CMAKE_BINARY_DIR}/_deps/zlib-build")
  endif ()
  set (_ps3_zfound FALSE)
  foreach (_ps3_zd ${_ps3_zincs})
    if (EXISTS "${_ps3_zd}/zlib.h")
      set (_ps3_zfound TRUE)
    endif ()
  endforeach ()
  if (NOT _ps3_zfound)
    message (FATAL_ERROR
      "PS3: zlib.h not found in ${_ps3_zincs} "
      "(skip_fetch=${_bennugd_skip_fetch_zlib_png})")
  endif ()
  message (STATUS "PS3 engine zlib includes: ${_ps3_zincs}")
  set (ZLIB_INCLUDE_DIRS "${_ps3_zincs}")
  include_directories (${_ps3_zincs})
  foreach (_ps3_zd ${_ps3_zincs})
    add_compile_options ("-I${_ps3_zd}")
  endforeach ()
endmacro ()
