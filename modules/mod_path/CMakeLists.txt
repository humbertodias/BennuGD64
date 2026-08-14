include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
)

add_library (
  mod_path
  ${MODULES}
  mod_path.c
) 

target_link_libraries (
  mod_path
  bgdrtm
  grbase
)
