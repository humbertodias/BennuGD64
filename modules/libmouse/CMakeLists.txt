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
  SDL2::SDL2
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
