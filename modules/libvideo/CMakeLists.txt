include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libgrbase
)

add_library (
  video
  ${MODULES}
  g_video.c
  g_regions.c
)

if (PLATFORM_PSP OR PSP)
  target_sources (video PRIVATE g_video_psp.c)
endif ()
if (PLATFORM_VITA OR VITA)
  target_sources (video PRIVATE g_video_vita.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (video PRIVATE g_video_ps2.c)
endif ()
if (PLATFORM_PS3 OR PS3)
  target_sources (video PRIVATE g_video_ps3.c)
endif ()
if (NINTENDO_SWITCH)
  target_sources (video PRIVATE g_video_switch.c)
endif ()
if (PLATFORM_DREAMCAST OR DREAMCAST)
  target_sources (video PRIVATE g_video_dc.c)
endif ()
if (PLATFORM_PANDORA OR OPENPANDORA)
  target_sources (video PRIVATE g_video_pandora.c)
endif ()
if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (video PRIVATE g_video_wii.c)
endif ()
if (EMSCRIPTEN)
  target_sources (video PRIVATE g_video_emscripten.c)
endif ()
if (WIN32)
  target_sources (video PRIVATE g_video_win32.c)
endif ()

add_dependencies ( video bennugd_git_header )

target_link_libraries (
  video
  bgdrtm
  grbase
  ${BENNUGD_SDL3_TARGET}
  ZLIB::ZLIB
)
