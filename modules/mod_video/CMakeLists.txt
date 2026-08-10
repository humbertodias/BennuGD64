include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../librender
  ../libvideo
  ../libgrbase
)

add_library (
  mod_video
  ${MODULES}
  mod_video.c
) 

target_link_libraries (
  mod_video
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  render
  video
  grbase
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_video
    DESTINATION
    /usr/lib
  )
endif ()
