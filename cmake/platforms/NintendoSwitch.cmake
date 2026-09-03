# Nintendo Switch homebrew: static modules, interpreter only (bgdi.elf → .nro).
# Compile .prg on a host with the switch-host preset.

set (STATIC_MODULES ON CACHE BOOL "Switch homebrew is a single NRO" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "NRO ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "Switch builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for libnx)")
endif ()
