include_directories (
  ../../core/include
  ../libkey
)

add_library (
  mod_key
  ${MODULES}
  mod_key.c
) 

target_link_libraries (
  mod_key
  key
  bgdrtm
)
