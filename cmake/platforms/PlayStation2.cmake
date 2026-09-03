# PlayStation 2 homebrew: static modules, interpreter only (bgdi.elf).
# Compile .prg on a host with the ps2-host preset.

set (STATIC_MODULES ON CACHE BOOL "PS2 homebrew is a single ELF" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "ELF ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "PS2 builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for ps2dev)")
endif ()
