include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libjoy
)

add_library (
  mod_joy
  ${MODULES}
  mod_joy.c
) 

target_link_libraries (
  mod_joy
  joy
  bgdrtm
)
