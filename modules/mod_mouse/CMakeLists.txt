include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  mod_mouse
  ${MODULES}
  mod_mouse.c
) 

target_link_libraries (
  mod_mouse
)
