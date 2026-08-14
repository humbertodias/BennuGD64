add_definitions (-D__BGDRTM__)
add_definitions (-DDEBUG)

include_directories (
  ../include
  ../../include
)

if (USE_LIBDES)
  include_directories (
    ../../3rdparty/des-4.04b
  )
endif ()

add_library (
  bgdrtm
  ${MODULES}
  fmath.c
  dcbr.c
  sysprocs.c
  strings.c
  dirs.c
  copy.c
  misc.c
  instance.c
  interpreter.c
  varspace_file.c
  bgd_lowmem.c
  ../../common/debug.c
  ../../common/b_crypt.c
  ../../common/files.c
  ../../common/xctype.c
)

if (USE_LIBDES)
  target_link_libraries (
    bgdrtm
    ZLIB::ZLIB
    des  
  )
else ()
  target_link_libraries (
    bgdrtm
    ZLIB::ZLIB
    OpenSSL::Crypto
  )
endif ()

if (UNIX)
  target_link_libraries ( bgdrtm m )
endif ()
if (NOT STATIC_MODULES)
  target_link_libraries ( bgdrtm ${CMAKE_DL_LIBS} )
endif ()

if (STATIC_MODULES)
  target_link_libraries (
    bgdrtm
    grbase
    bgload
    blit
    draw
    video
    render
    font
    text
    sdlhandler
    wm
    key
    mouse
    joy
    scroll
    mod_scroll
    mod_mouse
    mod_map
    mod_video
    mod_screen
    mod_blendop
    mod_text
    mod_draw
    mod_grproc
    mod_effects
    mod_key
    mod_cd
    mod_dir
    mod_file
    mod_crypt
    mod_joy
    mod_math
    mod_mem
    mod_proc
    mod_rand
    mod_regex
    mod_say
    mod_sort
    mod_sound
    mod_string
    mod_sys
    mod_time
    mod_timers
    mod_path
    mod_wm
    mod_debug
    mod_flic
    mod_m7    
  )
endif ()
