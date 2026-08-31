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
endif ()
