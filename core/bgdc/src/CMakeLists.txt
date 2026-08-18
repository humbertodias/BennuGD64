add_definitions (-D__BGDC__)

include_directories (
  ../include
  ../../include
)

if (STATIC_MODULES)
  include_directories (
    ../../../modules/libgrbase
    ../../../modules/libvideo
    ../../../modules/libblit
    ../../../modules/librender
    ../../../modules/libfont
    ../../../modules/libtext
    ../../../modules/libwm
    ../../../modules/libkey
    ../../../modules/libmouse
    ../../../modules/libjoy
    ../../../modules/libscroll
    ../../../modules/mod_scroll
    ../../../modules/mod_mouse
    ../../../modules/mod_map
    ../../../modules/mod_video
    ../../../modules/mod_screen
    ../../../modules/mod_blendop
    ../../../modules/mod_text
    ../../../modules/mod_draw
    ../../../modules/mod_grproc
    ../../../modules/mod_effects
    ../../../modules/mod_key
    ../../../modules/mod_cd
    ../../../modules/mod_dir
    ../../../modules/mod_file
    ../../../modules/mod_crypt
    ../../../modules/mod_joy
    ../../../modules/mod_math
    ../../../modules/mod_mem
    ../../../modules/mod_proc
    ../../../modules/mod_rand
    ../../../modules/mod_regex
    ../../../modules/mod_say
    ../../../modules/mod_sort
    ../../../modules/mod_sound
    ../../../modules/mod_string
    ../../../modules/mod_sys
    ../../../modules/mod_time
    ../../../modules/mod_timers
    ../../../modules/mod_path
    ../../../modules/mod_wm
    ../../../modules/mod_debug
    ../../../modules/mod_flic
    ../../../modules/mod_m7
  )
endif ()

add_executable (
  bgdc
  main.c
  main_core.c
  token.c
  identifier.c
  segment.c
  constants.c
  strings.c
  varspace.c
  procedure.c
  codeblock.c
  c_main.c
  c_data.c
  c_code.c
  c_debug.c
  dcbw.c
  error.c
  sysstub.c
  typedef.c
  ../../common/debug.c
  ../../common/b_crypt.c
  ../../common/files.c
  ../../common/xctype.c
)

include (${CMAKE_SOURCE_DIR}/cmake/FilesNative.cmake)
bennugd_files_native (bgdc)

add_dependencies ( bgdc bennugd_git_header )

if (USE_LIBDES)
  target_link_libraries (
    bgdc
    ZLIB::ZLIB
    des
  )
else ()
  target_link_libraries (
    bgdc
    ZLIB::ZLIB
    OpenSSL::Crypto
  )
endif ()

if (WIN32)
  target_link_libraries (bgdc shlwapi)
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "WASI")
  set_target_properties (bgdc PROPERTIES SUFFIX ".wasm")
endif ()
