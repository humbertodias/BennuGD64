include_directories (
  ../../core/include
)

add_library (
  grbase
  ${MODULES}
  g_bitmap.c
  g_clear.c
  g_grlib.c
  g_pal.c
  g_blendop.c
  g_conversion.c
  libgrbase.c
  bgd_handles.c
) 

target_link_libraries (
  grbase
)
