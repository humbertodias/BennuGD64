# PlayStation 5 homebrew: static modules, interpreter only (bgdi.elf payload).
# Compile .prg on a host with the ps5-host preset.
#
# SDL3 provides software surfaces and codecs. Bennu drives sceVideoOut and
# scePad directly. Audio output stays off; the mixer exports remain.

set (STATIC_MODULES ON CACHE BOOL "PS5 homebrew is a single ELF" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "ELF ships the interpreter; compile .prg on a host" FORCE)
set (BENNUGD_PS5_DUMMY_AUDIO ON CACHE BOOL
  "Keep mod_sound exports but disable PS5 audio output")

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
  message (WARNING "PS5 builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for the payload SDK)")
endif ()

if (DEFINED ENV{PS5_PAYLOAD_SDK} AND IS_DIRECTORY "$ENV{PS5_PAYLOAD_SDK}/target/lib")
  link_directories ("$ENV{PS5_PAYLOAD_SDK}/target/lib")
endif ()

set (THREADS_HAVE_PTHREAD_ARG FALSE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_RPATH TRUE CACHE BOOL "" FORCE)
set (CMAKE_SKIP_INSTALL_RPATH TRUE CACHE BOOL "" FORCE)
