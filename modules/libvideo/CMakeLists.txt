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

add_dependencies ( video bennugd_git_header )

target_link_libraries (
  video
  bgdrtm
  grbase
  ${BENNUGD_SDL3_TARGET}
)
