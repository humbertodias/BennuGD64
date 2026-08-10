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
  SDL2::SDL2
  bgdrtm
  render
  blit
  grbase
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_flic
    DESTINATION
    /usr/lib
  )
endif ()
