include_directories (
  ../../core/include
)

add_library (
  mod_string
  ${MODULES}
  mod_string.c
) 

target_link_libraries (
  mod_string
  bgdrtm
)
