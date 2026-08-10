include_directories (
  ../../core/include
)

add_library (
  key
  ${MODULES}
  libkey.c
) 

target_link_libraries (
  key
  SDL2::SDL2
  bgdrtm
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    key
    DESTINATION
    /usr/lib
  )
endif ()