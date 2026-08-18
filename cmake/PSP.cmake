# PlayStation Portable homebrew: static modules, interpreter only (bgdi.elf → EBOOT.PBP).
# Compile .prg on a host with the psp-host preset.

set (STATIC_MODULES ON CACHE BOOL "PSP homebrew is a single ELF/PBP" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "PBP ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT USE_LIBDES)
  message (WARNING "PSP builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for pspdev)")
endif ()
