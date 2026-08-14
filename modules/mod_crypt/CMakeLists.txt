include_directories (
  ../../core/include
)

add_library (
  mod_crypt
  ${MODULES}
  mod_crypt.c
) 

target_link_libraries (
  mod_crypt
  bgdrtm
)
