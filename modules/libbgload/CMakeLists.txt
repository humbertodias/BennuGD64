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
  ${BENNUGD_SDL3_TARGET}
)
