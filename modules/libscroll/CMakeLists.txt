include_directories (
  ../../core/include
  ../libblit
  ../libvideo
  ../libgrbase
  ../librender
)

add_library (
  scroll
  ${MODULES}
  libscroll.c
) 

target_link_libraries (
  scroll
  SDL3::SDL3
  bgdrtm
  render
  blit
  video
  grbase
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    scroll
    DESTINATION
    /usr/lib
  )
endif ()
