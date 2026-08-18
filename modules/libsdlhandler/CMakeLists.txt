include_directories (
  ../../core/include
)

add_library (
  sdlhandler
  ${MODULES}
  libsdlhandler.c
)

if (EMSCRIPTEN)
  target_sources (sdlhandler PRIVATE libsdlhandler_emscripten.c)
endif () 

target_link_libraries (
  sdlhandler
  ${BENNUGD_SDL3_TARGET}
)
