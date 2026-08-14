include_directories (
  ../../core/include
  ../libgrbase
)

add_library (
  mod_file
  ${MODULES}
  mod_file.c
) 

target_link_libraries (
  mod_file
  bgdrtm
  grbase
)
