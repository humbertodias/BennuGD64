include_directories (
  ../../core/include
  ../libvideo
)

add_library (
  key
  ${MODULES}
  libkey.c
)

if (PLATFORM_PSP OR PSP)
  target_sources (key PRIVATE libkey_psp.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (key PRIVATE libkey_ps2.c)
endif ()
if (EMSCRIPTEN)
  target_sources (key PRIVATE libkey_emscripten.c)
endif ()
if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (key PRIVATE libkey_wii.c)
endif ()

target_link_libraries (
  key
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  video
)
