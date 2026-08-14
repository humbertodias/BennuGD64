include_directories (
  ../../core/include
)

add_library (
  mod_rand
  ${MODULES}
  mod_rand.c
) 

target_link_libraries (
  mod_rand
)
