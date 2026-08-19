# Nintendo Wii homebrew: static modules, interpreter only (bgdi.elf → boot.dol).
# Compile .prg on a host with the wii-host preset.

set (STATIC_MODULES ON CACHE BOOL "Wii homebrew is a single DOL" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "DOL ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "Wii builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for libogc)")
endif ()
