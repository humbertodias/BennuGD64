include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
)

add_library (
  mod_cd
  ${MODULES}
  mod_cd.c
) 

target_link_libraries (
  mod_cd
  bgdrtm
  grbase
)
