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

if (PLATFORM_PS2 OR PS2)
  target_sources (mod_map PRIVATE file_fpg_ps2.c)
endif ()

target_link_libraries (
  mod_map
  PNG::PNG
  ${BENNUGD_SDL3_TARGET}
  bgload
  bgdrtm
  render
  font
  blit
  video
  grbase
)
