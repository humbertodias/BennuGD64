# PlayStation Vita homebrew: static modules, interpreter only (bgdi → eboot.bin / VPK).
# Compile .prg on a host with the vita-host preset.

set (STATIC_MODULES ON CACHE BOOL "Vita homebrew is a single VPK" FORCE)
set (INTERPRETER_ONLY ON CACHE BOOL "VPK ships the interpreter; compile .prg on a host" FORCE)

set (SDL_SHARED OFF CACHE BOOL "" FORCE)
set (SDL_STATIC ON CACHE BOOL "" FORCE)
set (SDL_TEST OFF CACHE BOOL "" FORCE)
set (SDL_TESTS OFF CACHE BOOL "" FORCE)
set (SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

if (NOT USE_LIBDES)
  message (WARNING "Vita builds should use -DUSE_LIBDES=ON (OpenSSL is not fetched for vitasdk)")
endif ()

# SoRR-vita: larger app memory class + parental flag in param.sfo.
set (VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1 -d ATTRIBUTE2=12")

if (DEFINED ENV{VITASDK} AND EXISTS "$ENV{VITASDK}/share/vita.cmake")
  include ("$ENV{VITASDK}/share/vita.cmake")
endif ()
