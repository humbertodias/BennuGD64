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
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
)
