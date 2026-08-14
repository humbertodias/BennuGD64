include_directories (
  ../../core/include
)

add_library (
  mod_sort
  ${MODULES}
  mod_sort.c
) 

target_link_libraries (
  mod_sort
  bgdrtm
)
