include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libbgload
  ../librender
  ../libfont
  ../libblit
  ../libvideo
  ../libgrbase
)

add_library (
  mod_map
  ${MODULES}
  file_pcx.c
  file_png.c
  file_map.c
  file_pal.c
  file_fpg.c
  file_fnt.c
  mod_map.c
) 

target_link_libraries (
  mod_map
  PNG::PNG
  SDL2::SDL2
  bgload
  bgdrtm
  render
  font
  blit
  video
  grbase
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_map
    DESTINATION
    /usr/lib
  )
endif ()
