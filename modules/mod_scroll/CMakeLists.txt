include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libscroll
  ../libvideo
  ../libgrbase
)

add_library (
  mod_scroll
  ${MODULES}
  mod_scroll.c
) 

target_link_libraries (
  mod_scroll
  scroll
)
