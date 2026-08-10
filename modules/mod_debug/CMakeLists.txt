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
  SDL2::SDL2
  bgdrtm
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_debug
    DESTINATION
    /usr/lib
  )
endif ()
