include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
)

add_library (
  font
  ${MODULES}
  libfont.c
) 

target_link_libraries (
  font
  grbase
)
