include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  joy
  ${MODULES}
  libjoy.c
) 

target_link_libraries (
  joy
  SDL2::SDL2
  bgdrtm
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    joy
    DESTINATION
    /usr/lib
  )
endif ()