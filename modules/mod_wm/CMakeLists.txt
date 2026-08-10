include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libvideo
  ../libgrbase
)

add_library (
  mod_wm
  ${MODULES}
  mod_wm.c
) 

target_link_libraries (
  mod_wm
  SDL3::SDL3
  bgdrtm
  video
  grbase
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_wm
    DESTINATION
    /usr/lib
  )
endif ()
