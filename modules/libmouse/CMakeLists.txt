include_directories (
  ../../core/include
  ../libdraw
  ../librender
  ../libblit
  ../libvideo
  ../libgrbase
)

add_library (
  mouse
  ${MODULES}
  libmouse.c
) 

if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (mouse PRIVATE libmouse_wii.c)
endif ()

target_link_libraries (
  mouse
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  draw
  render
  blit
  video
  grbase
)
