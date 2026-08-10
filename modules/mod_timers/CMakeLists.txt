include_directories (
  ../../core/include
)

add_library (
  mod_timers
  ${MODULES}
  mod_timers.c
) 

target_link_libraries (
  mod_timers
  SDL2::SDL2
  bgdrtm
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    mod_timers
    DESTINATION
    /usr/lib
  )
endif ()
