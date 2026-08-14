include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../librender
  ../libblit
  ../libvideo
  ../libgrbase
)

add_library (
  mod_m7
  ${MODULES}
  mod_m7.c
) 

target_link_libraries (
  mod_m7
  bgdrtm
  video
  render
  grbase
)
