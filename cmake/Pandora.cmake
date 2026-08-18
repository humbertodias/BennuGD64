# OpenPandora homebrew: static modules, interpreter only (bgdi → .pnd).
# Compile .prg on a host with the pandora-host preset.

set (STATIC_MODULES ON CACHE BOOL "Pandora homebrew is a single bgdi binary" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "PND ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)

if (NOT USE_LIBDES)
  message (WARNING "Pandora builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for Ångström)")
endif ()
