include_directories (
  ../../core/include
  ../libvideo
)

add_library (
  key
  ${MODULES}
  libkey.c
) 

target_link_libraries (
  key
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  video
)

if (NOT STATIC_MODULES)
  install (
    TARGETS
    key
    DESTINATION
    /usr/lib
  )
endif ()