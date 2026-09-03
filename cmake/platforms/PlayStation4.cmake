# PlayStation 4 homebrew: static modules, interpreter only (bgdi → eboot.bin / PKG).
# Compile .prg on a host with the ps4-host preset.
#
# Official SDL3 for PS4 is NDA-only. Homebrew OpenOrbis ships SDL2. Point
# BENNUGD_SDL3_PS4_REPO / BENNUGD_SDL3_PS4_REF (or FETCHCONTENT_SOURCE_DIR_SDL3)
# at NDA sources or a community SDL3 Orbis tree when available.

set (STATIC_MODULES ON CACHE BOOL "PS4 homebrew is a single PKG" FORCE)
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
set (SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "PS4 builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for OpenOrbis)")
endif ()

if (DEFINED ENV{OO_PS4_TOOLCHAIN} AND IS_DIRECTORY "$ENV{OO_PS4_TOOLCHAIN}")
  link_directories ("$ENV{OO_PS4_TOOLCHAIN}/lib")
endif ()

# Cached CMAKE_C_FLAGS from a prior configure will not pick up toolchain
# FLAGS_INIT. Keep FreeBSD→musl compat headers, -fshort-wchar, and Linux view.
set (_ps4_compat "${CMAKE_SOURCE_DIR}/cmake/toolchains/ps4-compat")
if (IS_DIRECTORY "${_ps4_compat}" AND NOT CMAKE_C_FLAGS MATCHES "ps4-compat")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${_ps4_compat}" CACHE STRING "" FORCE)
endif ()
if (IS_DIRECTORY "${_ps4_compat}" AND NOT CMAKE_CXX_FLAGS MATCHES "ps4-compat")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${_ps4_compat}" CACHE STRING "" FORCE)
endif ()
if (NOT CMAKE_C_FLAGS MATCHES "fshort-wchar")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fshort-wchar" CACHE STRING "" FORCE)
endif ()
if (NOT CMAKE_CXX_FLAGS MATCHES "fshort-wchar")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fshort-wchar" CACHE STRING "" FORCE)
endif ()
if (NOT CMAKE_C_FLAGS MATCHES "U__FreeBSD__")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U__FreeBSD__ -U__FreeBSD_kernel__ -D__linux__=1" CACHE STRING "" FORCE)
endif ()
if (NOT CMAKE_CXX_FLAGS MATCHES "U__FreeBSD__")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U__FreeBSD__ -U__FreeBSD_kernel__ -D__linux__=1" CACHE STRING "" FORCE)
endif ()

# ld.lld rejects clang-style -pthread / -Wl,-rpath from FindThreads / RPATH.
set (THREADS_HAVE_PTHREAD_ARG FALSE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_RPATH TRUE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_INSTALL_RPATH TRUE CACHE BOOL "" FORCE)
# Deduplicate EXE_LINKER_FLAGS if FLAGS_INIT was applied onto a cached value.
if (DEFINED ENV{OO_PS4_TOOLCHAIN} AND IS_DIRECTORY "$ENV{OO_PS4_TOOLCHAIN}")
  set (_ps4_ld
    "-m elf_x86_64 -pie --script $ENV{OO_PS4_TOOLCHAIN}/link.x --eh-frame-hdr"
    " -L$ENV{OO_PS4_TOOLCHAIN}/lib $ENV{OO_PS4_TOOLCHAIN}/lib/crt1.o")
  string (CONCAT _ps4_ld ${_ps4_ld})
  set (CMAKE_EXE_LINKER_FLAGS "${_ps4_ld}" CACHE STRING "" FORCE)
endif ()
