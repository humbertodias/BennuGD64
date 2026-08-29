include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
)

add_library (
  blit
  ${MODULES}
  libblit.c
  g_blit.c
  g_pixel.c
)

if (PLATFORM_PS2 OR PS2)
  target_sources (blit PRIVATE g_blit_ps2.c)
endif ()

target_link_libraries (
  blit
  bgdrtm
  grbase
)
