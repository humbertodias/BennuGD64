include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  bgload
  ${MODULES}
  bgload.c
) 

target_link_libraries (
  bgload
  bgdrtm
  SDL2::SDL2
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    bgload
    DESTINATION
    /usr/lib
  )
endif ()