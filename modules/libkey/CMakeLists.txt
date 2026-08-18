include_directories (
  ../../core/include
  ../libvideo
)

add_library (
  key
  ${MODULES}
  libkey.c
)

if (PLATFORM_PSP OR PSP)
  target_sources (key PRIVATE libkey_psp.c)
endif ()

target_link_libraries (
  key
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  video
)
