include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libkey
  ../librender
  ../libgrbase
)

add_library (
  mod_debug
  ${MODULES}
  mod_debug.c
  systexts.c
) 

target_link_libraries (
  mod_debug
  key
  render
  grbase
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
)
