include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libbgload
)

add_library (
  mod_sound
  ${MODULES}
  mod_sound.c
) 

if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (mod_sound PRIVATE mod_sound_wii.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (mod_sound PRIVATE mod_sound_ps2.c)
endif ()

target_link_libraries (
  mod_sound
  ${BENNUGD_SDL3_TARGET}
  bgload
  bgdrtm
  grbase
  ${BENNUGD_SDL3_MIXER_TARGET}
)
