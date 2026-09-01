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
if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (sdlhandler PRIVATE libsdlhandler_wii.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (sdlhandler PRIVATE libsdlhandler_ps2.c)
endif ()
if (PLATFORM_VITA OR VITA)
  target_sources (sdlhandler PRIVATE libsdlhandler_vita.c)
endif ()
if (PLATFORM_PS3 OR PS3)
  target_sources (sdlhandler PRIVATE libsdlhandler_ps3.c)
endif () 

target_link_libraries (
  sdlhandler
  ${BENNUGD_SDL3_TARGET}
)
