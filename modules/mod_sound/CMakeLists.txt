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
if (PLATFORM_PS3 OR PS3)
  target_sources (mod_sound PRIVATE mod_sound_ps3.c)
endif ()
if (PLATFORM_PS4 OR PS4)
  target_sources (mod_sound PRIVATE mod_sound_ps4.c)
  if (BENNUGD_PS4_DUMMY_AUDIO)
    target_compile_definitions (mod_sound PRIVATE MODSOUND_PS4_DUMMY_AUDIO=1)
  endif ()
endif ()
if (PLATFORM_XBOX360 OR XBOX360)
  target_sources (mod_sound PRIVATE mod_sound_xbox360.c)
endif ()

target_link_libraries (
  mod_sound
  ${BENNUGD_SDL3_TARGET}
  bgload
  bgdrtm
  grbase
  ${BENNUGD_SDL3_MIXER_TARGET}
)
