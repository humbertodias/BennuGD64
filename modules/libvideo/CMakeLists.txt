include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
)

add_library (
  video
  ${MODULES}
  g_video.c
  g_regions.c
) 

target_link_libraries (
  video
  bgdrtm
  grbase
  SDL2::SDL2
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    video
    DESTINATION
    /usr/lib
  )
endif ()
