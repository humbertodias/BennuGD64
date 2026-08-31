add_definitions (-D__LIBRENDER)

include_directories (
  ../../core/include
  ../../core/bgdrtm/include
  ../libblit
  ../libvideo
  ../libgrbase
)

add_library (
  render
  ${MODULES}
  librender.c
  g_fade.c
  g_frame.c
  g_stats.c
  g_instance.c
  g_object.c
  g_rects.c
  g_screen.c
  scaler_hq2x.c
  scaler_normal.c
  scaler_scale2x.c
  scaler_scanline.c
)

if (PLATFORM_PSP OR PSP)
  target_sources (render PRIVATE g_frame_psp.c)
endif ()
if (PLATFORM_VITA OR VITA)
  target_sources (render PRIVATE g_frame_vita.c)
endif ()
if (PLATFORM_PS2 OR PS2)
  target_sources (render PRIVATE g_frame_ps2.c)
endif ()
if (NINTENDO_WII OR PLATFORM_WII)
  target_sources (render PRIVATE g_frame_wii.c)
endif ()

target_link_libraries (
  render
  ${BENNUGD_SDL3_TARGET}
  bgdrtm
  blit
  video
  grbase
)
