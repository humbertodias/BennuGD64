include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  wm
  ${MODULES}
  libwm.c
) 

target_link_libraries (
  wm
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
)
