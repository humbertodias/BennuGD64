include_directories (
  ../../core/include
  ../../core/bgdrtm/include
)

add_library (
  mod_wpad
  ${MODULES}
  mod_wpad.c
)

target_link_libraries (
  mod_wpad
  bgdrtm
)

if (NOT NINTENDO_WII AND NOT PLATFORM_WII)
  target_link_libraries (
    mod_wpad
    ${BENNUGD_SDL3_TARGET}
  )
endif ()
