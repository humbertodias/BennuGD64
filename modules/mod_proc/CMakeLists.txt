include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  mod_proc
  ${MODULES}
  mod_proc.c
) 

target_link_libraries (
  mod_proc
  bgdrtm
)
