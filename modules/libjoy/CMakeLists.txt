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
  SDL3::SDL3
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