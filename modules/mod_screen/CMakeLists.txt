include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libvideo
  ../librender
  ../libblit
  ../libgrbase
  ../libscroll
)

add_library (
  mod_screen
  ${MODULES}
  mod_screen.c
) 

target_link_libraries (
  mod_screen
  bgdrtm
  video
  render
  blit
  grbase
)
