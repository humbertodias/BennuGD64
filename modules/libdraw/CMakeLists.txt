include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
  ../libblit
)

add_library (
  draw
  ${MODULES}
  libdraw.c
) 

target_link_libraries (
  draw
  grbase
  blit
)
