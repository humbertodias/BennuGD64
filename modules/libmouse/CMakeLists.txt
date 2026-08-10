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

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mouse
    DESTINATION
    /usr/lib
  )
endif ()
