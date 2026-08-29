include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  joy
  ${MODULES}
  libjoy.c
)

if (EMSCRIPTEN)
  target_sources (joy PRIVATE libjoy_emscripten.c)
endif ()

if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (joy PRIVATE libjoy_wii.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (joy PRIVATE libjoy_ps2.c)
endif () 

target_link_libraries (
  joy
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
)
