include_directories (
  ../../core/include
  ../librender
  ../libblit
  ../libgrbase
)

add_library (
  mod_flic
  ${MODULES}
  mod_flic.c
) 

target_link_libraries (
  mod_flic
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  render
  blit
  grbase
)
