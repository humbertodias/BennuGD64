include_directories (
  ../../core/include
)

add_library (
  mod_sys
  ${MODULES}
  mod_sys.c
) 

target_link_libraries (
  mod_sys
  bgdrtm
)
