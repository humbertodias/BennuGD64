# Sega Dreamcast homebrew: static modules, interpreter only (bgdi.elf → .cdi).
# Compile .prg on a host with the dreamcast-host preset.

set (STATIC_MODULES ON CACHE BOOL "Dreamcast homebrew is a single ELF/CDI" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "CDI ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "Dreamcast builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for KallistiOS)")
endif ()
