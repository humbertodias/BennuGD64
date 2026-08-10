include_directories (
  ../../core/include
)

add_library (
  mod_time
  ${MODULES}
  mod_time.c
) 

target_link_libraries (
  mod_time
  SDL2::SDL2
  bgdrtm
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_time
    DESTINATION
    /usr/lib
  )
endif ()
