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
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  render
  blit
  video
  grbase
)
